#ifndef TRIMWRIGHT_H
#define TRIMWRIGHT_H

#ifndef TRIMWRIGHT_MAX_STATE_DEPTH
    #define TRIMWRIGHT_MAX_STATE_DEPTH 6
#endif

#include <stdint.h>

namespace TrimWright {


    //----------------------------------------------------------------------
    // Basic supporting data types (and macros)
    //

    typedef enum {
        SIG_SUPER = 0,  // state (in an HSM) should report its super-state
        SIG_ENTER,      // state was entered
        SIG_LEAVE,      // state is about to be left
        SIG_INIT,       // state should take its initial transition
        SIG_IDLE,       // pseudo-event dispatched if no other events
        SIG_USER = 0x10 // start of user-defined signal numbers
    } Signal;

    struct Event {
        uint8_t signal;
    };


    // Returned by a state as the outcome of dispatching an event to the state.
    // It is much better to use the macros below, for example:
    //      return TW_HANDLED();
    enum DispatchOutcome {
        DISPATCH_HANDLED,
        DISPATCH_UNHANDLED,
        DISPATCH_TRANSITION,
        DISPATCH_SUPER
    };

    // Returned by a state to report that it has handled the event.
    #define TW_HANDLED()        (TrimWright::DISPATCH_HANDLED)

    // Returned by a state to report that it has not handled the event.
    // This is most useful inside complex conditional structurs in guards.
    // Otherwise it is slightly better to return TW_SUPER() instead.
    // States should never return this when handling SIG_ENTER or SIG_LEAVE.
    #define TW_UNHANDLED()      (TrimWright::DISPATCH_UNHANDLED)

    // Returned by a state to transition to another state.
    // States should never return this when handling SIG_ENTER or SIG_LEAVE.
    #define TW_TRANSITION(s)    ((m_stateTemp = TrimWright::State(s)), TrimWright::DISPATCH_TRANSITION)

    // Returned by a state (in an HSM) to report the parent state.
    // This should definitely be returned for SIG_SUPER, but is also generally
    // returned for any unhandled event.
    #define TW_SUPER(s)         ((m_stateTemp = TrimWright::State(s)), TrimWright::DISPATCH_SUPER)



    //----------------------------------------------------------------------
    // Finite State Machine class
    // This is a good choice if the state machine is "flat" (not hierarchical).
    //

    class FSM;
    typedef DispatchOutcome (FSM::* State)(const Event* event);


    class FSM {
        protected:
            static Event    PSEUDOEVENTS[5];
            State           m_stateCurrent;
            State           m_stateTemp;
            friend void dispatchIdle(FSM*);

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



    //----------------------------------------------------------------------
    // Hierarchical State Machine class
    // This is a good choice if the states in the state machine are
    // hierarchical.
    //

    class HSM : public FSM {
        protected:
            // root of the state hierarchy
            // top-level states of the application should report this as their
            // super states via TW_SUPER((State) &HSM::stateROOT).
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



    //----------------------------------------------------------------------
    // Event Queue
    // This is handy if it's a difficult or unnatural for your sketch to
    // dispatch the event as soon as it is generated.
    //

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



    //----------------------------------------------------------------------
    // "Sugar" Functions
    // These are necessary but might be handy.
    //

    // Dispatches an event (of the Event class) with the SIG_IDLE signal.
    void dispatchIdle(FSM* machine);


    // Dispatches all events in the queue to the state machine.
    // If `idleIfEmpty` is true and there are no events in the queue
    // then a SIG_IDLE event will be dispatched to the state machine.
    void dispatchAll(FSM* machine, IQueue* queue, bool idleIfEmpty);

};


#endif
