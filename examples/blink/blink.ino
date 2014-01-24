
#include <Arduino.h>
#include <TrimWright.h>
using namespace TrimWright;


enum {
    SIG_TIMER = SIG_USER
};


class Blink : public FSM {
    public:
        DispatchOutcome stateON(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    digitalWrite(13, HIGH);
                    return TW_HANDLED();
                case SIG_TIMER:
                    return TW_TRANSITION(&Blink::stateOFF);
                default:
                    return TW_HANDLED();
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    digitalWrite(13, LOW);
                    return TW_HANDLED();
                case SIG_TIMER:
                    return TW_TRANSITION(&Blink::stateON);
                default:
                    return TW_HANDLED();
            }
        }
} blink;


const uint32_t TIMEOUT = 500;   // milliseconds
uint32_t timeout;


void setup() {
    pinMode(13, OUTPUT);
    timeout = millis() + TIMEOUT;
    blink.init((State) &Blink::stateON);
}


void loop() {
    uint32_t now = millis();
    if (now >= timeout) {
        timeout = now + TIMEOUT;
        Event event;
        event.signal = (Signal) SIG_TIMER;
        blink.dispatch(&event);
    }
}


