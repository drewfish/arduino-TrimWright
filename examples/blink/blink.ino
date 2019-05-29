/*
Copyright 2019 Drew Folta <drew@folta.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Arduino.h>
#include <TrimWright.h>
using namespace TrimWright;


enum {
    SIG_TIMER = SIG_USER
};


class LED : public FSM {
    public:
        void setup() {
            pinMode(13, OUTPUT);
            init((State) &LED::stateON);
        }

        DispatchOutcome stateON(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    // when we enter this state, turn on the LED
                    digitalWrite(13, HIGH);
                    return TW_HANDLED();
                case SIG_TIMER:
                    return TW_TRANSITION(&LED::stateOFF);
                default:
                    return TW_HANDLED();
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    // when we enter this state, turn off the LED
                    digitalWrite(13, LOW);
                    return TW_HANDLED();
                case SIG_TIMER:
                    return TW_TRANSITION(&LED::stateON);
                default:
                    return TW_HANDLED();
            }
        }
} led;


const uint32_t TIMEOUT = 500;   // milliseconds
uint32_t nextTimeout;


void setup() {
    nextTimeout = millis() + TIMEOUT;
    led.setup();
}


void loop() {
    uint32_t now = millis();
    // In this example we'll create a periodic event by hand.
    // In a real project you might want to use a timer or have
    // a changing pin (e.g. Arduino `attachInterrupt`) create
    // events.
    if (now >= nextTimeout) {
        nextTimeout = now + TIMEOUT;
        Event event;
        event.signal = (Signal) SIG_TIMER;
        led.dispatch(&event);
    }
}


