#include <stdint.h>
#include <iostream>
using namespace std;

#include "../../DFActors.h"
using namespace DFActors;


class Hardware {
    private:
        uint32_t m_loops;   // number of times loop() has been called
    public:
        void setup();
        void loop();
        void setLED(bool on);
        void sleep();
} hw;


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


class Blink : public Actor {
    protected:
        void debugDispatch(const Event* event, const char* stateName) {
            cout << "dispatch " << signalName(event->signal) << " to state " << stateName << endl;
        }

        DispatchOutcome stateROOT(const Event* event) {
            debugDispatch(event, "ROOT");
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
            debugDispatch(event, "ON");
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
            debugDispatch(event, "OFF");
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
    cout << "------hw setup----" << endl;
    m_loops = 0;
}

void Hardware::loop() {
    cout << "------hw loop---- " << m_loops << endl;
    ++m_loops;
    if (0 == m_loops % 3) {
        DFACTORS_EVENT_CLASS event;
        event.signal = (Signal) SIG_TIMER;
        blink.dispatch(&event);
    }
}

void Hardware::setLED(bool on) {
    cout << "------hw setLED---- " << (on ? "ON" : "OFF") << endl;
}

void Hardware::sleep() {
    // nothing to do
    cout << "------hw sleep----" << endl;
}


int main(int argc, const char* argv[]) {
    // setup
    hw.setup();
    blink.init();

    // loop
    for (int i = 0; i < 10; i++) {
        hw.loop();
    }
}


