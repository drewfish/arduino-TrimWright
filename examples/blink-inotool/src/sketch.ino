
#include <Arduino.h>
#include <DFActors.h>
using namespace DFActors;


enum {
    SIG_TIMER = SIG_USER
};


class Blink : public FSM {
    public:
        DispatchOutcome stateON(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    digitalWrite(13, HIGH);
                    return FSM_HANDLED();
                case SIG_TIMER:
                    return FSM_TRANSITION(&Blink::stateOFF);
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            switch (event->signal) {
                case SIG_ENTER:
                    digitalWrite(13, LOW);
                    return FSM_HANDLED();
                case SIG_TIMER:
                    return FSM_TRANSITION(&Blink::stateON);
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


