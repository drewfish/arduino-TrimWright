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

#include <TrimWright.h>

enum {
    SIG_DOWN = TrimWright::SIG_USER,
    SIG_UP,
};

class Blinker : public TrimWright::FSM {
    private:
        uint16_t delayOn;
        uint16_t delayOff;
    public:
        void setup() {
            init((TrimWright::State) &Blinker::stateOFF);
        }
        void blink() {
            if (delayOn) {
                digitalWrite(13, HIGH);
                delay(delayOn);
                digitalWrite(13, LOW);
            }
            if (delayOff) {
                delay(delayOff);
            }
        }
        TrimWright::DispatchOutcome stateOFF(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case TrimWright::SIG_ENTER:
                    delayOn = 0;
                    delayOff = 0;
                    return TW_HANDLED();
                case SIG_DOWN:
                    // wraps around to fast
                    return TW_TRANSITION(&Blinker::stateFAST);
                case SIG_UP:
                    return TW_TRANSITION(&Blinker::stateSLOW);
                default:
                    return TW_HANDLED();
            }
        }
        TrimWright::DispatchOutcome stateSLOW(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case TrimWright::SIG_ENTER:
                    delayOn = 100;
                    delayOff = 400;
                    return TW_HANDLED();
                case SIG_DOWN:
                    return TW_TRANSITION(&Blinker::stateOFF);
                case SIG_UP:
                    return TW_TRANSITION(&Blinker::stateFAST);
                default:
                    return TW_HANDLED();
            }
        }
        TrimWright::DispatchOutcome stateFAST(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case TrimWright::SIG_ENTER:
                    delayOn = 100;
                    delayOff = 100;
                    return TW_HANDLED();
                case SIG_DOWN:
                    return TW_TRANSITION(&Blinker::stateSLOW);
                case SIG_UP:
                    // wraps around to off
                    return TW_TRANSITION(&Blinker::stateOFF);
                default:
                    return TW_HANDLED();
            }
        }
} blinker;

void ISR_Button_Up() {
    static TrimWright::Event evt;
    evt.signal = SIG_UP;
    blinker.dispatch(&evt);
}

void ISR_Button_Down() {
    static TrimWright::Event evt;
    evt.signal = SIG_DOWN;
    blinker.dispatch(&evt);
}

void setup() {
    blinker.setup();
    pinMode(11, INPUT_PULLDOWN);
    pinMode(12, INPUT_PULLDOWN);
    pinMode(13, OUTPUT);
    attachInterrupt(11, ISR_Button_Up, RISING);
    attachInterrupt(12, ISR_Button_Down, RISING);
}

void loop() {
    blinker.blink();
}

