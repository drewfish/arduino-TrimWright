
#include "DFActors.h"


namespace DFActors {



    //----------------------------------------------------------------------
    // FSM
    //

    Event FSM::PSEUDOEVENTS[5] = {
        { SIG_SUPER },  // not needed by FSM but used by HSM
        { SIG_ENTER },
        { SIG_LEAVE },
        { SIG_INIT },
        { SIG_IDLE }
    };
    #define _FSM_PSEUDO(state,sig) ((this->*(state))(&(PSEUDOEVENTS[sig])))


    FSM::FSM() : m_stateCurrent(0), m_stateTemp(0) {
        // nothing to do
    }


    void
    FSM::init(State initial) {
        m_stateCurrent = initial;
        _FSM_PSEUDO(m_stateCurrent, SIG_ENTER);
        while (DISPATCH_TRANSITION == _FSM_PSEUDO(m_stateCurrent, SIG_INIT)) {
            _FSM_PSEUDO(m_stateCurrent, SIG_LEAVE);
            this->m_stateCurrent = this->m_stateTemp;
            _FSM_PSEUDO(m_stateCurrent, SIG_ENTER);
        }
        this->m_stateTemp = 0;
    }


    void
    FSM::dispatch(const Event* event) {
        if (DISPATCH_TRANSITION == (this->*m_stateCurrent)(event)) {
            _FSM_PSEUDO(m_stateCurrent, SIG_LEAVE);
            this->init(this->m_stateTemp);
        }
    }



    //----------------------------------------------------------------------
    // HSM
    //

    void
    HSM::init(State initial) {
        m_stateCurrent = (State) &HSM::stateROOT;
        m_stateTemp = initial;
        DispatchOutcome out = DISPATCH_TRANSITION;
        while (DISPATCH_TRANSITION == out) {
            State source = m_stateCurrent;  // the transition "from" state
            State target = m_stateTemp;     // the transition "to" state
            State path[DFACTORS_MAX_STATE_DEPTH];
            int8_t p = 0;
            for (p = 0; m_stateTemp && (m_stateTemp != source); p++) {
                path[p] = m_stateTemp;
                _FSM_PSEUDO(m_stateTemp, SIG_SUPER);
            }
            for (; p > 0; p--) {
                _FSM_PSEUDO(path[p-1], SIG_ENTER);
            }
            m_stateCurrent = target;
            out = _FSM_PSEUDO(m_stateCurrent, SIG_INIT);
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
                out = _FSM_PSEUDO(m_stateTemp, SIG_SUPER);
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
            if (DISPATCH_HANDLED == _FSM_PSEUDO(m_stateTemp, SIG_LEAVE)) {
                _FSM_PSEUDO(m_stateTemp, SIG_SUPER);
            }
        }
        m_stateCurrent = source;

        // transition to self
        if (source == target) {
            // All we need to do is leave-and-enter the state.
            _FSM_PSEUDO(source, SIG_LEAVE);
            _FSM_PSEUDO(target, SIG_ENTER);
            m_stateCurrent = target;
        }
        else {
            // fill path with list of target supers (including target)
            p = 0;
            m_stateTemp = target;
            do {
                path[p] = m_stateTemp;
                out = _FSM_PSEUDO(m_stateTemp, SIG_SUPER);
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
                    if (DISPATCH_HANDLED == _FSM_PSEUDO(m_stateCurrent, SIG_LEAVE)) {
                        _FSM_PSEUDO(m_stateCurrent, SIG_SUPER);
                    }
                    m_stateCurrent = m_stateTemp;
                }
            } while (m_stateCurrent);

            // drill down into the target
            if (-1 != enter_start) {
                for (p = enter_start; p >= 0; p--) {
                    _FSM_PSEUDO(path[p], SIG_ENTER);
                    m_stateCurrent = path[p];
                }
            }
        } // not a self-transition

        // Handle the initial transition(s) of the target state
        while (DISPATCH_TRANSITION == _FSM_PSEUDO(m_stateCurrent, SIG_INIT)) {
            source = m_stateCurrent;    // the transition "from" state
            target = m_stateTemp;       // the transition "to" state
            p = 0;
            for (p = 0; m_stateTemp && (m_stateTemp != source); p++) {
                path[p] = m_stateTemp;
                _FSM_PSEUDO(m_stateTemp, SIG_SUPER);
            }
            for (; p > 0; p--) {
                _FSM_PSEUDO(path[p-1], SIG_ENTER);
            }
            m_stateCurrent = target;
        }
    }


    DispatchOutcome
    HSM::stateROOT(const Event* event) {
        if (SIG_SUPER == event->signal) {
            return HSM_SUPER(0);
        }
        return FSM_HANDLED();
    }



    //----------------------------------------------------------------------
    // SUGAR FUNCTIONS
    //
    // TODO -- size_t dispatchAll()



};


