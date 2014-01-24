#include <stdint.h>
#include <iostream>
using namespace std;

#include "../../DFActors.h"
#include "../../DFActors.cpp"
using namespace DFActors;


char debugBuffer[32];
char* debugEvent(Event* event) {
    if (event) {
        snprintf(debugBuffer, 32, "sig=%d", event->signal);
    }
    else {
        snprintf(debugBuffer, 32, "NULL");
    }
    return debugBuffer;
}


void testRingBuffer() {
    cout << "------------------------------------- RING BUFFER" << endl;
    QueueRingBuffer<Event, 5> q;
    Event e;

    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 10, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 11, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 12, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 13, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 14, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 15, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    cout << endl;

    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    cout << endl;

    e.signal = 16, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    e.signal = 17, q.push_back(&e);
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    q.pop_front();
    cout << "size " << int(q.size()) << " -- front " << debugEvent(q.front()) << endl;
    cout << endl;
}


int main(int argc, const char* argv[]) {
    testRingBuffer();
}


