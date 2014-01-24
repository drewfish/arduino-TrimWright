//
// g++ -o main main.cpp
//


#include <iostream>
using namespace std;

#include "../../DFActors.h"
#include "../../DFActors.cpp"


enum {
    SIG_A = DFActors::SIG_USER,
    SIG_B,
    SIG_C,
    SIG_D,
    SIG_E,
    SIG_F,
    SIG_G,
    SIG_H,
    SIG_I,
    SIG_TERMINATE
};


const char* signalName(uint8_t sig) {
    if (sig == DFActors::SIG_SUPER) { return "SUPER"; }
    if (sig == DFActors::SIG_ENTER) { return "ENTER"; }
    if (sig == DFActors::SIG_LEAVE) { return "LEAVE"; }
    if (sig == DFActors::SIG_INIT)  { return "INIT"; }
    if (sig == DFActors::SIG_IDLE)  { return "IDLE"; }
    if (sig == SIG_A)   { return "A"; }
    if (sig == SIG_B)   { return "B"; }
    if (sig == SIG_C)   { return "C"; }
    if (sig == SIG_D)   { return "D"; }
    if (sig == SIG_E)   { return "E"; }
    if (sig == SIG_F)   { return "F"; }
    if (sig == SIG_G)   { return "G"; }
    if (sig == SIG_H)   { return "H"; }
    if (sig == SIG_I)   { return "I"; }
    if (sig == SIG_TERMINATE) { return "TERMINATE"; }
    return "???";
}
void debugDispatch(const DFActors::Event* event, const char* stateName) {
    //if (DFActors::SIG_SUPER == event->signal) { return; }
    cout << stateName << "-" << (signalName(event->signal)) << ";";
}


class Test : public DFActors::HSM {
    public:
        uint8_t foo;

        void post(char e) {
            DFActors::Event event;
            event.signal = DFActors::SIG_USER + (e - 'A');
            this->dispatch(&event);
        }

        void terminate() {
            DFActors::Event event;
            event.signal = SIG_TERMINATE;
            this->dispatch(&event);
        }

        DFActors::DispatchOutcome stateS(const DFActors::Event* event) {
            const char* NAME = "s";
            switch (event->signal) {
                case DFActors::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_INIT:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS11);
                case SIG_E:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS11);
                case SIG_I:
                    if (this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 0;
                        return HSM_HANDLED();
                    }
                    else {
                        return HSM_UNHANDLED();
                    }
                case SIG_TERMINATE:
                    exit(0);
            }
            return HSM_SUPER(&Test::stateROOT);
        }

        DFActors::DispatchOutcome stateS1(const DFActors::Event* event) {
            const char* NAME = "s1";
            switch (event->signal) {
                case DFActors::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_INIT:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS11);
                case SIG_A:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS1);
                case SIG_B:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS11);
                case SIG_C:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS2);
                case SIG_D:
                    if (!this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 1;
                        return HSM_TRANSITION(&Test::stateS);
                    }
                    else {
                        return HSM_UNHANDLED();
                    }
                    break;
                case SIG_F:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS211);
                case SIG_I:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
            }
            return HSM_SUPER(&Test::stateS);
        }

        DFActors::DispatchOutcome stateS11(const DFActors::Event* event) {
            const char* NAME = "s11";
            switch (event->signal) {
                case DFActors::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case SIG_D:
                    if (this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 0;
                        return HSM_TRANSITION(&Test::stateS1);
                    }
                    else {
                        return HSM_UNHANDLED();
                    }
                    break;
                case SIG_G:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS211);
                case SIG_H:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS);
            }
            return HSM_SUPER(&Test::stateS1);
        }

        DFActors::DispatchOutcome stateS2(const DFActors::Event* event) {
            const char* NAME = "s2";
            switch (event->signal) {
                case DFActors::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_INIT:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS211);
                case SIG_C:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS1);
                case SIG_F:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS11);
                case SIG_I:
                    if (!this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 1;
                        return HSM_HANDLED();
                    }
                    else {
                        return HSM_UNHANDLED();
                    }
                    break;
            }
            return HSM_SUPER(&Test::stateS);
        }

        DFActors::DispatchOutcome stateS21(const DFActors::Event* event) {
            const char* NAME = "s21";
            switch (event->signal) {
                case DFActors::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_INIT:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS211);
                case SIG_A:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS21);
                case SIG_B:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS211);
                case SIG_G:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS1);
            }
            return HSM_SUPER(&Test::stateS2);
        }

        DFActors::DispatchOutcome stateS211(const DFActors::Event* event) {
            const char* NAME = "s211";
            switch (event->signal) {
                case DFActors::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case DFActors::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return HSM_HANDLED();
                case SIG_D:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS21);
                case SIG_H:
                    debugDispatch(event, NAME);
                    return HSM_TRANSITION(&Test::stateS);
            }
            return HSM_SUPER(&Test::stateS21);
        }

} test;


int main(int argc, const char* argv[]) {
    cout << "INIT:  ";
    test.init((DFActors::State) &Test::stateS2);
    cout << "   foo=" << int(test.foo) << endl;

    const char* EVENTS = "GIAD DCEE GII";
    for (int i = 0; EVENTS[i]; i++) {
        char event = EVENTS[i];
        if (' ' == event) {
            continue;
        }
        cout << event << ":  ";
        test.post(event);
        cout << "   foo=" << int(test.foo) << endl;
    }

    test.terminate();
}


