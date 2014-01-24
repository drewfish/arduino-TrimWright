#ifndef DFACTORS_H
#define DFACTORS_H

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


    // Returned by a state as the outcome of dispatching the event to the state.
    // It is much better to use the macros below, for example:
    //      return FSM_HANDLED();
    enum DispatchOutcome {
        DISPATCH_HANDLED,
        DISPATCH_UNHANDLED,
        DISPATCH_TRANSITION,
        DISPATCH_SUPER
    };

    // Returned by a state to report that it has handled the event.
    #define FSM_HANDLED()       (DFActors::DISPATCH_HANDLED)

    // Returned by a state to report that it has not handled the event.
    // This is more useful inside complex conditional structurs in guards.
    // Otherwise it is slightly better to return HSM_SUPER() instead.
    #define FSM_UNHANDLED()     (DFActors::DISPATCH_UNHANDLED)

    // Returned by a state to transition to another state.
    #define FSM_TRANSITION(s)   ((m_stateTemp = DFActors::State(s)), DFActors::DISPATCH_TRANSITION)

    // Returned by a state to report parent state.
    // This should definitely be returned for SIG_SUPER, but is also generally
    // returned for any unhandled event.
    #define HSM_SUPER(s)        ((m_stateTemp = DFActors::State(s)), DFActors::DISPATCH_SUPER)


    class FSM;
    typedef DispatchOutcome (FSM::* State)(const Event* event);


    class FSM {
        protected:
            static Event    PSEUDOEVENTS[5];
            State           m_stateCurrent;
            State           m_stateTemp;

        public:
            FSM();

            // This performs the transition to the first (initial) state.
            // This will following the "init" internal transition (iteratively,
            // if there are any).
            void init(State initial);

            // This dispatches an event to the current state.
            // It doesn't generally need to be overriden by child classes.
            void dispatch(const Event* event);
    };


    class HSM : public FSM {
        protected:
            // root of the state hierarchy
            // top-level states of the application should report this as their
            // super states via HSM_SUPER((State) &HSM::stateROOT).
            DispatchOutcome stateROOT(const Event* event);

        public:
            // This performs the transition to the first (initial) state.
            // This will following the "init" internal transition (iteratively,
            // if there are any).
            void init(State initial);

            // This dispatches an event to the current state.
            // It doesn't generally need to be overriden by child classes.
            void dispatch(const Event* event);
    };


    // interface for all queue implementations
    class IQueue {
        public:
            // adds the event to the end of the queue
            virtual void push_back(Event*) = 0;

            // returns the event at the front of the queue
            virtual Event* front() = 0;

            // removes the event at the front of the queue
            virtual void pop_front() = 0;

            // returns the number of events in the queue
            virtual uint8_t size() = 0;

            virtual ~IQueue() {}
    };


    // a queue implementation that stores (copies of) the events in a ring buffer
    template <class EventType, uint8_t MAX_EVENTS>
    class QueueRingBuffer : public IQueue {
        protected:
            EventType   m_buffer[MAX_EVENTS];
            uint8_t     m_front;
            uint8_t     m_back; // points one past the actual end
            uint8_t     m_size;

        public:

            QueueRingBuffer() : m_front(0), m_back(0), m_size(0) {
                // TODO:  needed?
                //memset((void*) m_buffer, 0, MAX_EVENTS * sizeof(EventType));
            }

            // copies the event into the ring buffer
            virtual void push_back(EventType* event) {
                if ((m_size + 1) > MAX_EVENTS) {
                    // no more room
                    return;
                }
                m_buffer[m_back] = *event;
                m_back++;
                if (m_back == MAX_EVENTS) {
                    // wrap around
                    m_back = 0;
                }
                m_size++;
            }

            virtual EventType* front() {
                if (!m_size) {
                    // nothing on the front
                    return 0;
                }
                return &(m_buffer[m_front]);
            }

            virtual void pop_front() {
                if (!m_size) {
                    // nothing to pop
                    return;
                }
                m_front++;
                if (m_front == MAX_EVENTS) {
                    // wrap around
                    m_front = 0;
                }
                m_size--;
            }

            virtual uint8_t size() {
                return m_size;
            }
    };


};


#endif
