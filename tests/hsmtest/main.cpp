// if building a real arduino program than no-op this file
#ifndef ARDUINO


#include <iostream>
using namespace std;

#include "../../src/TrimWright.h"
#include "../../src/TrimWright.cpp"


enum {
    SIG_A = TrimWright::SIG_USER,
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
    if (sig == TrimWright::SIG_SUPER) { return "SUPER"; }
    if (sig == TrimWright::SIG_ENTER) { return "ENTER"; }
    if (sig == TrimWright::SIG_LEAVE) { return "LEAVE"; }
    if (sig == TrimWright::SIG_INIT)  { return "INIT"; }
    if (sig == TrimWright::SIG_IDLE)  { return "IDLE"; }
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
void debugDispatch(const TrimWright::Event* event, const char* stateName) {
    //if (TrimWright::SIG_SUPER == event->signal) { return; }
    cout << stateName << "-" << (signalName(event->signal)) << ";";
}


class Test : public TrimWright::HSM {
    public:
        uint8_t foo;

        void post(char e) {
            TrimWright::Event event;
            event.signal = TrimWright::SIG_USER + (e - 'A');
            this->dispatch(&event);
        }

        void terminate() {
            TrimWright::Event event;
            event.signal = SIG_TERMINATE;
            this->dispatch(&event);
        }

        TrimWright::DispatchOutcome stateS(const TrimWright::Event* event) {
            const char* NAME = "s";
            switch (event->signal) {
                case TrimWright::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_INIT:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS11);
                case SIG_E:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS11);
                case SIG_I:
                    if (this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 0;
                        return TW_HANDLED();
                    }
                    else {
                        return TW_UNHANDLED();
                    }
                case SIG_TERMINATE:
                    exit(0);
            }
            return TW_SUPER(&Test::stateROOT);
        }

        TrimWright::DispatchOutcome stateS1(const TrimWright::Event* event) {
            const char* NAME = "s1";
            switch (event->signal) {
                case TrimWright::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_INIT:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS11);
                case SIG_A:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS1);
                case SIG_B:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS11);
                case SIG_C:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS2);
                case SIG_D:
                    if (!this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 1;
                        return TW_TRANSITION(&Test::stateS);
                    }
                    else {
                        return TW_UNHANDLED();
                    }
                    break;
                case SIG_F:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS211);
                case SIG_I:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
            }
            return TW_SUPER(&Test::stateS);
        }

        TrimWright::DispatchOutcome stateS11(const TrimWright::Event* event) {
            const char* NAME = "s11";
            switch (event->signal) {
                case TrimWright::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case SIG_D:
                    if (this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 0;
                        return TW_TRANSITION(&Test::stateS1);
                    }
                    else {
                        return TW_UNHANDLED();
                    }
                    break;
                case SIG_G:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS211);
                case SIG_H:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS);
            }
            return TW_SUPER(&Test::stateS1);
        }

        TrimWright::DispatchOutcome stateS2(const TrimWright::Event* event) {
            const char* NAME = "s2";
            switch (event->signal) {
                case TrimWright::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_INIT:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS211);
                case SIG_C:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS1);
                case SIG_F:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS11);
                case SIG_I:
                    if (!this->foo) {
                        debugDispatch(event, NAME);
                        this->foo = 1;
                        return TW_HANDLED();
                    }
                    else {
                        return TW_UNHANDLED();
                    }
                    break;
            }
            return TW_SUPER(&Test::stateS);
        }

        TrimWright::DispatchOutcome stateS21(const TrimWright::Event* event) {
            const char* NAME = "s21";
            switch (event->signal) {
                case TrimWright::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_INIT:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS211);
                case SIG_A:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS21);
                case SIG_B:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS211);
                case SIG_G:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS1);
            }
            return TW_SUPER(&Test::stateS2);
        }

        TrimWright::DispatchOutcome stateS211(const TrimWright::Event* event) {
            const char* NAME = "s211";
            switch (event->signal) {
                case TrimWright::SIG_ENTER:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case TrimWright::SIG_LEAVE:
                    debugDispatch(event, NAME);
                    return TW_HANDLED();
                case SIG_D:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS21);
                case SIG_H:
                    debugDispatch(event, NAME);
                    return TW_TRANSITION(&Test::stateS);
            }
            return TW_SUPER(&Test::stateS21);
        }

} test;


int main(int argc, const char* argv[]) {
    cout << "INIT:  ";
    test.init((TrimWright::State) &Test::stateS2);
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


#endif
