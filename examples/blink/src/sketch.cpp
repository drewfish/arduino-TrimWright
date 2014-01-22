
#include <Arduino.h>
#include <DFActors.h>
using namespace DFActors;


class Hardware {
    private:
        static const uint32_t TIMEOUT = 500; // milliseconds
        uint32_t timeout;
    public:
        void setup();
        void loop();
        void setLED(bool on);
        void sleep();
} hw;


enum {
    SIG_TIMER = SIG_USER
};


class Blink : public Actor {
    protected:
        DispatchOutcome stateROOT(const Event* event) {
            switch (event->signal) {
                case SIG_INIT:
                    return TRANSITION(&Blink::stateON);
                case SIG_SUPER:
                    return SUPER(NULL);
                default:
                    return HANDLED();
            }
        }

        DispatchOutcome stateON(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    hw.setLED(true);
                    return HANDLED();
                case SIG_IDLE:
                    hw.sleep();
                    return HANDLED();
                case SIG_TIMER:
                    return TRANSITION(&Blink::stateOFF);
                default:
                    return SUPER(&Blink::stateROOT);
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    hw.setLED(false);
                    return HANDLED();
                case SIG_IDLE:
                    hw.sleep();
                    return HANDLED();
                case SIG_TIMER:
                    return TRANSITION(&Blink::stateON);
                default:
                    return SUPER(&Blink::stateROOT);
            }
        }
} blink;


void Hardware::setup() {
    pinMode(13, OUTPUT);
    timeout = millis() + TIMEOUT;
}


void Hardware::loop() {
    uint32_t now = millis();
    if (now >= timeout) {
        timeout = now + TIMEOUT;
        DFACTORS_EVENT_CLASS event;
        event.signal = (Signal) SIG_TIMER;
        blink.dispatch(&event);
    }
}


void Hardware::setLED(bool on) {
    digitalWrite(13, (on ? HIGH : LOW));
}


void Hardware::sleep() {
    // FUTURE:  put into a low power mode that still is woken by the timer
    // used by millis()
}


void setup() {
    hw.setup();
    blink.init();
}


void loop() {
    hw.loop();
}


