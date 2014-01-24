#include <stdint.h>
#include <iostream>
using namespace std;

#include "../../DFActors.h"
#include "../../DFActors.cpp"
using namespace DFActors;


enum {
    SIG_A = SIG_USER,
    SIG_B,
    SIG_C,
    SIG_D,
    SIG_E,
    SIG_F,
    SIG_G,
    SIG_H,
    SIG_I,
    SIG_J
};


char buffer[32];
const char* signalName(uint8_t sig) {
    if (sig == SIG_SUPER) { return "SUPER"; }
    if (sig == SIG_ENTER) { return "ENTER"; }
    if (sig == SIG_LEAVE) { return "LEAVE"; }
    if (sig == SIG_INIT)  { return "INIT"; }
    if (sig == SIG_IDLE)  { return "IDLE"; }
    snprintf(buffer, 32, "sig=%c", ('A' + (sig - SIG_USER)));
    return buffer;
}


class StateMachine : public FSM {
    public:
        DispatchOutcome stateDEBUG(Event* event) {
            cout << "got " << signalName(event->signal) << endl;
            return FSM_HANDLED();
        }
};


void testDispatchIdle(FSM* machine) {
    cout << "---------------------------------------------- test dispatchIdle()" << endl;
    dispatchIdle(machine);
    cout << endl;
}


void testDispatchAll(FSM* machine) {
    QueueRingBuffer<Event, 5> queue;
    Event e;

    cout << "---------------------------------------------- test dispatchAll()" << endl;

    e.signal = SIG_A; queue.push_back(&e);
    e.signal = SIG_B; queue.push_back(&e);
    e.signal = SIG_C; queue.push_back(&e);
    e.signal = SIG_D; queue.push_back(&e);
    dispatchAll(machine, &queue, false);
    cout << "----" << endl;

    dispatchAll(machine, &queue, false);
    cout << "----" << endl;

    dispatchAll(machine, &queue, true);
    cout << "----" << endl;

    e.signal = SIG_E; queue.push_back(&e);
    e.signal = SIG_F; queue.push_back(&e);
    e.signal = SIG_G; queue.push_back(&e);
    e.signal = SIG_H; queue.push_back(&e);
    e.signal = SIG_I; queue.push_back(&e);
    e.signal = SIG_J; queue.push_back(&e);
    dispatchAll(machine, &queue, true);
    cout << "----" << endl;

    dispatchAll(machine, &queue, true);
    cout << "----" << endl;

    cout << endl;
}


int main(int argc, const char* argv[]) {
    StateMachine machine;
    machine.init((State) &StateMachine::stateDEBUG);

    testDispatchIdle(&machine);
    testDispatchAll(&machine);
}


