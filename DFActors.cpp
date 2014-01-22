
#include "DFActors.h"


namespace DFActors {


    //----------------------------------------------------------------------
    // QUEUE HEAP
    //
    // TODO -- implement this



    //----------------------------------------------------------------------
    // QUEUE HEAP
    //
    // TODO -- implement this



    //----------------------------------------------------------------------
    // HSM
    //

    Event HSM::PSEUDOEVENTS[5] = {
        { SIG_SUPER },
        { SIG_ENTER },
        { SIG_LEAVE },
        { SIG_INIT },
        { SIG_IDLE }
    };


    HSM::HSM() : m_stateCurrent(&HSM::stateROOT), m_stateTemp(0) {
        // nothing to do
    }


    void
    HSM::init() {
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


    void
    HSM::dispatch(const Event* event) {
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
        // witout having to repeat the HSM_SUPER(). This is so that the
        // HSM_SUPER() is written in one place in the state method, which
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


    DispatchOutcome
    HSM::stateROOT(const Event* event) {
        if (SIG_SUPER == event->signal) {
            return HSM_SUPER(0);
        }
        return HSM_HANDLED();
    }


};


