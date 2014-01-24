
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


class Blink : public HSM {
    public:
        DispatchOutcome stateON(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    hw.setLED(true);
                    return FSM_HANDLED();
                case SIG_IDLE:
                    hw.sleep();
                    return FSM_HANDLED();
                case SIG_TIMER:
                    return FSM_TRANSITION(&Blink::stateOFF);
                default:
                    return HSM_SUPER(&Blink::stateROOT);
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    hw.setLED(false);
                    return FSM_HANDLED();
                case SIG_IDLE:
                    hw.sleep();
                    return FSM_HANDLED();
                case SIG_TIMER:
                    return FSM_TRANSITION(&Blink::stateON);
                default:
                    return HSM_SUPER(&Blink::stateROOT);
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
        Event event;
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
    blink.init((State) &Blink::stateON);
}


void loop() {
    hw.loop();
}


