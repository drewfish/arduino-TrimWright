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

#define TIMEOUT_HOLD_MS (1000)
#define TIMEOUT_REPEAT_MS (200)

enum {
    SIG_TIMER = TrimWright::SIG_USER,
    SIG_BUTTON_DOWN,
    SIG_BUTTON_UP,
};

class Timer {
    private:
        uint32_t count;
        uint32_t compare;   // 0 means timer isn't running
    public:
        void setup() {
            count = 0;
            compare = 0;
        }
        void run(int16_t ms) {
            count = 0;
            compare = ms;
        }
        void stop() {
            compare = 0;
        }
        void tick();    // implemented below
} timer;

class LED {
    private:
        const uint8_t MAX_SPEED = 8;
        uint8_t speed;
        uint32_t count;
        uint32_t compareOn;
        uint32_t compareOff;
        void setCompares() {
            compareOn = 30 * speed;
            compareOff = compareOn + (20 * speed);
        }
    public:
        void setup() {
            pinMode(13, OUTPUT);
            digitalWrite(13, LOW);
            speed = 1;
            count = 0;
            setCompares();
        }
        void changeSpeed(bool faster) {
            if (faster) {
                speed = min(speed + 1, MAX_SPEED);
            } else {
                speed = max(speed - 1, 1);
            }
            setCompares();
        }
        void tick() {
            count++;
            if (count >= compareOff) {
                digitalWrite(13, LOW);
                count = 0;
                return;
            }
            if (count >= compareOn) {
                digitalWrite(13, HIGH);
                return;
            }
        }
} led;

class Button : public TrimWright::HSM {
    public:
        TrimWright::QueueRingBuffer<TrimWright::Event, 6> queue;
        void setup();   // implemented below
        void tick() {
            TrimWright::dispatchAll(this, &queue, false);
        }
        void click() {
            led.changeSpeed(true);
        }
        void hold() {
            led.changeSpeed(false);
        }
        TrimWright::DispatchOutcome stateUP(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case SIG_BUTTON_DOWN:
                    return TW_TRANSITION(&Button::stateDOWN);
                default:
                    return TW_SUPER(&Button::stateROOT);
            }
        }
        TrimWright::DispatchOutcome stateDOWN(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case TrimWright::SIG_INIT:
                    return TW_TRANSITION(&Button::stateHOLDING);
                case TrimWright::SIG_LEAVE:
                    timer.stop();
                    return TW_HANDLED();
                case SIG_BUTTON_UP:
                    return TW_TRANSITION(&Button::stateUP);
                default:
                    return TW_SUPER(&Button::stateROOT);
            }
        }
        TrimWright::DispatchOutcome stateHOLDING(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case TrimWright::SIG_ENTER:
                    timer.run(TIMEOUT_HOLD_MS);
                    return TW_HANDLED();
                case SIG_TIMER:
                    hold();
                    return TW_TRANSITION(&Button::stateREPEATING);
                case SIG_BUTTON_UP:
                    click();
                    // bubble-up event handling to our parent
                    return TW_UNHANDLED();
                default:
                    return TW_SUPER(&Button::stateDOWN);
            }
        }
        TrimWright::DispatchOutcome stateREPEATING(const TrimWright::Event* evt) {
            switch (evt->signal) {
                case TrimWright::SIG_ENTER:
                    timer.run(TIMEOUT_REPEAT_MS);
                    return TW_HANDLED();
                case SIG_TIMER:
                    hold();
                    return TW_HANDLED();
                default:
                    return TW_SUPER(&Button::stateDOWN);
            }
        }
} button;

void Timer::tick() {
    if (!compare) {
        // timer isn't running
        return;
    }
    count++;
    if (count == compare) {
        TrimWright::Event evt;
        evt.signal = SIG_TIMER;
        button.queue.push_back(&evt);
        count = 0;
    }
}

void ISR_Button_Change() {
    TrimWright::Event evt;
    if (digitalRead(12) == LOW) {
        evt.signal = SIG_BUTTON_DOWN;
    } else {
        evt.signal = SIG_BUTTON_UP;
    }
    button.queue.push_back(&evt);
}

void Button::setup() {
    pinMode(12, INPUT_PULLUP);
    bool down = digitalRead(12) == LOW;
    if (down) {
        init((TrimWright::State) &Button::stateDOWN);
    } else {
        init((TrimWright::State) &Button::stateUP);
    }
    attachInterrupt(12, ISR_Button_Change, CHANGE);
}

void setup() {
    led.setup();
    timer.setup();
    button.setup();
}

void loop() {
    // we'll run the loop once every millisecond (roughly)
    // (A more sophisticated approach would be to put the CPU to sleep, but
    // first setup a hardware timer to wake the CPU every millisecond.)
    delay(1);
    led.tick();
    timer.tick();
    button.tick();
}
