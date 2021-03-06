// if building a real arduino program than no-op this file
#ifndef ARDUINO


#include <stdint.h>
#include <iostream>
using namespace std;

#include "../../src/TrimWright.h"
#include "../../src/TrimWright.cpp"
using namespace TrimWright;


enum {
    SIG_TIMER = SIG_USER
};


const char* signalName(uint8_t sig) {
    if (sig == SIG_SUPER) { return "SUPER"; }
    if (sig == SIG_ENTER) { return "ENTER"; }
    if (sig == SIG_LEAVE) { return "LEAVE"; }
    if (sig == SIG_INIT)  { return "INIT"; }
    if (sig == SIG_IDLE)  { return "IDLE"; }
    if (sig == SIG_TIMER) { return "TIMER"; }
    return "???";
}


class Blink : public FSM {
    public:
        void debugDispatch(const Event* event, const char* stateName) {
            cout << "dispatch " << signalName(event->signal) << " to state " << stateName << endl;
        }

        DispatchOutcome stateON(const Event* event) {
            debugDispatch(event, "ON");
            switch (event->signal) {
                case SIG_ENTER:
                    cout << "------setLED---- ON" << endl;
                    return TW_HANDLED();
                case SIG_TIMER:
                    return TW_TRANSITION(&Blink::stateOFF);
                default:
                    return TW_HANDLED();
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            debugDispatch(event, "OFF");
            switch (event->signal) {
                case SIG_ENTER:
                    cout << "------setLED---- OFF" << endl;
                    return TW_HANDLED();
                case SIG_TIMER:
                    return TW_TRANSITION(&Blink::stateON);
                default:
                    return TW_HANDLED();
            }
        }
} blink;


int main(int argc, const char* argv[]) {
    // setup
    blink.init((State) &Blink::stateON);

    // loop
    uint32_t loops = 0;
    for (int i = 0; i < 10; i++) {
        cout << "------loop---- " << loops << endl;
        ++loops;
        if (0 == loops % 3) {
            Event event;
            event.signal = (Signal) SIG_TIMER;
            blink.dispatch(&event);
        }
    }
}


#endif
