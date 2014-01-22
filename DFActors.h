/*
TODO
    DFActors::IQueue interface ?
        virtual void push_back(DFACTORS_EVENT_CLASS* event) = 0;
        virtual DFACTORS_EVENT_CLASS* front() = 0;
        virtual void release(DFACTORS_EVENT_CLASS* event) = 0;

*/


#ifndef DFACTORS_H
#define DFACTORS_H

#ifndef DFACTORS_EVENT_CLASS
    #define DFACTORS_EVENT_CLASS DFActors::Event
#endif
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
        Signal  signal; // this field should exist any event class which is used
                        // and should have the above values (possibly more)
        // optional payload
    };

    class QueueHeap {
        // TODO -- implement these
        public:
            void push_back(DFACTORS_EVENT_CLASS* event);
            DFACTORS_EVENT_CLASS* front();
            void release(DFACTORS_EVENT_CLASS* event);
    };

    class QueueRingBuffer {
        // TODO -- implement this
    };

    enum DispatchOutcome {
        DISPATCH_HANDLED,
        DISPATCH_UNHANDLED,
        DISPATCH_TRANSITION,
        DISPATCH_SUPER
    };

    class Actor;
    typedef DispatchOutcome (Actor::* State)(const DFACTORS_EVENT_CLASS* event);

    class Actor {
        public:
            DFACTORS_QUEUE_CLASS    m_queue;
        protected:
            static DFACTORS_EVENT_CLASS PSEUDOEVENTS[5];
            State                   m_stateCurrent;
            State                   m_stateTemp;

        public:
            Actor() : m_stateCurrent(&Actor::stateROOT), m_stateTemp(0) {}

            // This is called when a state is the target of a transition.
            // This will following the "init" internal transition (iteratively,
            // if there are any).
            // TODO -- remove noinline
            void __attribute__ ((noinline)) init() {
                while (DISPATCH_TRANSITION == (this->*m_stateCurrent)(&(PSEUDOEVENTS[SIG_INIT]))) {
                    State source = m_stateCurrent;  // the transition "from" state
                    State target = m_stateTemp;     // the transition "to" state
                    State path[DFACTORS_MAX_STATE_DEPTH];
                    int8_t p = 0;
                    for (p = 0; m_stateTemp && (m_stateTemp != source); p++) {
                        path[p] = m_stateTemp;
                        (this->*m_stateTemp)(&(PSEUDOEVENTS[SIG_SUPER]));
                    }
                    for (; p > 0; p--) {
                        (this->*(path[p-1]))(&(PSEUDOEVENTS[SIG_ENTER]));
                    }
                    m_stateCurrent = target;
                }
            }

            // called to dispatch an event
            // this doesn't generally need to be overriden by child classes
            // TODO -- remove noinline
            void __attribute__ ((noinline)) dispatch(const DFACTORS_EVENT_CLASS* event) {
                State source;           // the state that initiated the transition
                State target;           // the transition "to" state
                DispatchOutcome out;
                State path[DFACTORS_MAX_STATE_DEPTH];
                int8_t p, path_end, enter_start;

                // When dispatching the event the DispatchOutcome can be
                // one of HANDLED, UNHANDLED, TRANSITION, or SUPER.
                // UNHANDLED is usually returned on a branching handler that
                // might also return HANDLED. A call to SUPER should be made
                // to figure out where the event should bubble up to.
                // (The primary purpose of the UNHANDLED outcome is to facilitate
                // authoring of sophisticated guard logic within a switch case
                // witout having to repeat the SUPER(). This is so that the
                // SUPER() is written in one place in the state method, which
                // is less error-prone should it need to be updated.)
                m_stateTemp = m_stateCurrent;
                do {
                    source = m_stateTemp;
                    out = (this->*m_stateTemp)(event);
                    if (out == DISPATCH_UNHANDLED) {
                        out = (this->*m_stateTemp)(&(PSEUDOEVENTS[SIG_SUPER]));
                    }
                } while (out == DISPATCH_SUPER);

                if (out != DISPATCH_TRANSITION) {
                    // bail early
                    return;
                }

                target = m_stateTemp;

                // exit current state to source of transition
                m_stateTemp = m_stateCurrent;
                while (m_stateTemp && (m_stateTemp != source)) {
                    if (DISPATCH_HANDLED == (this->*m_stateTemp)(&(PSEUDOEVENTS[SIG_LEAVE]))) {
                        (this->*m_stateTemp)(&(PSEUDOEVENTS[SIG_SUPER]));
                    }
                }
                m_stateCurrent = source;

                // transition to self
                if (source == target) {
                    // All we need to do is leave-and-enter the state.
                    (this->*source)(&(PSEUDOEVENTS[SIG_LEAVE]));
                    (this->*target)(&(PSEUDOEVENTS[SIG_ENTER]));
                    m_stateCurrent = target;
                }
                else {
                    // fill path with list of target supers (including target)
                    p = 0;
                    m_stateTemp = target;
                    do {
                        path[p] = m_stateTemp;
                        out = (this->*m_stateTemp)(&(PSEUDOEVENTS[SIG_SUPER]));
                        p++;
                    } while (m_stateTemp && (out == DISPATCH_SUPER));
                    path_end = p;

                    // find least common ancestor (LCA)
                    enter_start = -1;
                    do {
                        if (m_stateCurrent == target) {
                            // target is a super of the state initiated the transition
                            break;
                        }
                        else {
                            // see if current state is in the list of supers of the target
                            for (p = 0; p < path_end; p++) {
                                if (path[p] == m_stateCurrent) {
                                    enter_start = p - 1;
                                    break;
                                }
                            }
                            if (-1 != enter_start) {
                                // found it!
                                break;
                            }
                            // leave this state and enter the super state
                            if (DISPATCH_HANDLED == (this->*m_stateCurrent)(&(PSEUDOEVENTS[SIG_LEAVE]))) {
                                (this->*m_stateCurrent)(&(PSEUDOEVENTS[SIG_SUPER]));
                            }
                            m_stateCurrent = m_stateTemp;
                        }
                    } while (m_stateCurrent);

                    // drill down into the target
                    if (-1 != enter_start) {
                        for (p = enter_start; p >= 0; p--) {
                            (this->*(path[p]))(&(PSEUDOEVENTS[SIG_ENTER]));
                            m_stateCurrent = path[p];
                        }
                    }
                } // not a self-transition

                // We want to do this->init() to handle initial transition(s)
                // of the target state.  We'll inline the algorithm here so
                // that we can reuse existing stack variables.
                while (DISPATCH_TRANSITION == (this->*m_stateCurrent)(&(PSEUDOEVENTS[SIG_INIT]))) {
                    source = m_stateCurrent;    // the transition "from" state
                    target = m_stateTemp;       // the transition "to" state
                    p = 0;
                    for (p = 0; m_stateTemp && (m_stateTemp != source); p++) {
                        path[p] = m_stateTemp;
                        (this->*m_stateTemp)(&(PSEUDOEVENTS[SIG_SUPER]));
                    }
                    for (; p > 0; p--) {
                        (this->*(path[p-1]))(&(PSEUDOEVENTS[SIG_ENTER]));
                    }
                    m_stateCurrent = target;
                }
            }

            // dispatch all events or call SIG_IDLE pseudo-event
            void dispatchAll() {
                // TODO -- implement these methods first
                /*
                DFACTORS_EVENT_CLASS* e;
                while (e = m_queue.front()) {
                    dispatch(e);
                    m_queue.release(e);
                }
                // TODO -- call IDLE if no events
                */
            }

        protected:

            #define HANDLED() (DISPATCH_HANDLED)

            #define UNHANDLED() (DISPATCH_UNHANDLED)

            // call in states to transition to another state
            #define TRANSITION(s) ((m_stateTemp = State(s)), DISPATCH_TRANSITION)

            // call in states to transition to another state
            #define SUPER(s) ((m_stateTemp = State(s)), DISPATCH_SUPER)

            // root of the state hierarchy
            virtual DispatchOutcome stateROOT(const DFACTORS_EVENT_CLASS* event) {
                if (SIG_SUPER == event->signal) {
                    return SUPER(NULL);
                }
                return HANDLED();
            }
    };
    DFACTORS_EVENT_CLASS Actor::PSEUDOEVENTS[5] = {
        { SIG_SUPER },
        { SIG_ENTER },
        { SIG_LEAVE },
        { SIG_INIT },
        { SIG_IDLE }
    };

};


#endif
