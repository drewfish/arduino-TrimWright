#include <stdint.h>
#include <iostream>
using namespace std;

#include "../../DFActors.h"
#include "../../DFActors.cpp"
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


class Blink : public HSM {
    public:
        void debugDispatch(const Event* event, const char* stateName) {
            cout << "dispatch " << signalName(event->signal) << " to state " << stateName << endl;
        }

        DispatchOutcome stateON(const Event* event) {
            debugDispatch(event, "ON");
            switch (event->signal) {
                case SIG_ENTER:
                    hw.setLED(true);
                    return HSM_HANDLED();
                case SIG_IDLE:
                    hw.sleep();
                    return HSM_HANDLED();
                case SIG_TIMER:
                    return HSM_TRANSITION(&Blink::stateOFF);
                default:
                    return HSM_SUPER(&Blink::stateROOT);
            }
        }

        DispatchOutcome stateOFF(const Event* event) {
            debugDispatch(event, "OFF");
            switch (event->signal) {
                case SIG_ENTER:
                    hw.setLED(false);
                    return HSM_HANDLED();
                case SIG_IDLE:
                    hw.sleep();
                    return HSM_HANDLED();
                case SIG_TIMER:
                    return HSM_TRANSITION(&Blink::stateON);
                default:
                    return HSM_SUPER(&Blink::stateROOT);
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
        Event event;
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
    blink.init((State) &Blink::stateON);

    // loop
    for (int i = 0; i < 10; i++) {
        hw.loop();
    }
}


