#ifndef DFACTORS_H
#define DFACTORS_H

#ifndef DFACTORS_QUEUE_CLASS
    #define DFACTORS_QUEUE_CLASS DFActors::QueueHeap
#endif
#ifndef DFACTORS_MAX_STATE_DEPTH
    #define DFACTORS_MAX_STATE_DEPTH 6
#endif

#include <stdint.h>

namespace DFActors {


    typedef enum {
        SIG_SUPER = 0,  // state should report its super-state
        SIG_ENTER,      // state was entered
        SIG_LEAVE,      // state is about to be left
        SIG_INIT,       // state should take its initial transition
        SIG_IDLE,       // pseudo-event dispatched if no other events
        SIG_USER = 0x10 // start of user-defined signal numbers
    } Signal;

    struct Event {
        uint8_t signal;
    };


    // TODO -- IQueue interface


    class QueueHeap {
        // TODO -- implement these
        public:
            void push_back(Event* event);
            Event* front();
            void release(Event* event);
    };


    class QueueRingBuffer {
        // TODO -- implement this
    };


    // Returned by a state as the outcome of dispatching the event to the state.
    // It is much better to use the macros below, for example:
    //      return HSM_HANDLED();
    enum DispatchOutcome {
        DISPATCH_HANDLED,
        DISPATCH_UNHANDLED,
        DISPATCH_TRANSITION,
        DISPATCH_SUPER
    };

    // Returned by a state to report that it has handled the event.
    #define HSM_HANDLED()       (DISPATCH_HANDLED)

    // Returned by a state to report that it has not handled the event.
    // This is more useful inside complex conditional structurs in guards.
    // Otherwise it is slightly better to return HSM_SUPER() instead.
    #define HSM_UNHANDLED()     (DISPATCH_UNHANDLED)

    // Returned by a state to transition to another state.
    #define HSM_TRANSITION(s)   ((m_stateTemp = State(s)), DISPATCH_TRANSITION)

    // Returned by a state to report parent state.
    // This should definitely be returned for SIG_SUPER, but is also generally
    // returned for any unhandled event.
    #define HSM_SUPER(s)        ((m_stateTemp = State(s)), DISPATCH_SUPER)


    class HSM;
    typedef DispatchOutcome (HSM::* State)(const Event* event);


    class HSM {
        protected:
            static Event    PSEUDOEVENTS[5];
            State           m_stateCurrent;
            State           m_stateTemp;

        public:
            HSM();

            // This is called when a state is the target of a transition.
            // This will following the "init" internal transition (iteratively,
            // if there are any).
            void init();

            // called to dispatch an event
            // this doesn't generally need to be overriden by child classes
            void dispatch(const Event* event);

        protected:

            // root of the state hierarchy
            virtual DispatchOutcome stateROOT(const Event* event);

    };

};


#endif
