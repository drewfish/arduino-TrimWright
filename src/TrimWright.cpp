/*
Copyright 2019 Drew Folta <drew@folta.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "TrimWright.h"


namespace TrimWright {



    #define _TW_PSEUDO(state,sig) ((this->*(state))(&(PSEUDOEVENTS[sig])))



    //----------------------------------------------------------------------
    // FINITE STATE MACHINE
    //


    FSM::FSM() : m_stateCurrent(0), m_stateTemp(0) {
        // nothing to do
    }


    void
    FSM::TW_METHOD_INIT(State initial) {
        m_stateCurrent = initial;
        _TW_PSEUDO(m_stateCurrent, SIG_ENTER);
        while (DISPATCH_TRANSITION == _TW_PSEUDO(m_stateCurrent, SIG_INIT)) {
            _TW_PSEUDO(m_stateCurrent, SIG_LEAVE);
            this->m_stateCurrent = this->m_stateTemp;
            _TW_PSEUDO(m_stateCurrent, SIG_ENTER);
        }
        this->m_stateTemp = 0;
    }


    void
    FSM::dispatch(const Event* event) {
        if (DISPATCH_TRANSITION == (this->*m_stateCurrent)(event)) {
            _TW_PSEUDO(m_stateCurrent, SIG_LEAVE);
            this->TW_METHOD_INIT(this->m_stateTemp);
        }
    }


    void
    FSM::dispatchIdle() {
        this->dispatch(&(PSEUDOEVENTS[SIG_IDLE]));
    }


    void
    FSM::dispatchAll(IQueue* queue, bool idleIfEmpty) {
        if (queue->size()) {
            while (queue->size()) {
                this->dispatch(queue->front());
                queue->pop_front();
            }
        }
        else {
            if (idleIfEmpty) {
                this->dispatchIdle();
            }
        }
    }



    //----------------------------------------------------------------------
    // HIERARCHICAL STATE MACHINE
    //


    HSM::HSM() : m_stateCurrent(0), m_stateTemp(0) {
        // nothing to do
    }


    void
    HSM::TW_METHOD_INIT(State initial) {
        m_stateCurrent = (State) &HSM::stateROOT;
        m_stateTemp = initial;
        DispatchOutcome out = DISPATCH_TRANSITION;
        while (DISPATCH_TRANSITION == out) {
            State source = m_stateCurrent;  // the transition "from" state
            State target = m_stateTemp;     // the transition "to" state
            State path[TRIMWRIGHT_MAX_STATE_DEPTH];
            int8_t p = 0;
            for (p = 0; m_stateTemp && (m_stateTemp != source); p++) {
                path[p] = m_stateTemp;
                _TW_PSEUDO(m_stateTemp, SIG_SUPER);
            }
            for (; p > 0; p--) {
                _TW_PSEUDO(path[p-1], SIG_ENTER);
            }
            m_stateCurrent = target;
            out = _TW_PSEUDO(m_stateCurrent, SIG_INIT);
        }
    }


    void
    HSM::dispatch(const Event* event) {
        State source;           // the state that initiated the transition
        State target;           // the transition "to" state
        DispatchOutcome out;
        State path[TRIMWRIGHT_MAX_STATE_DEPTH];
        int8_t p, path_end, enter_start;

        // When dispatching the event the DispatchOutcome can be
        // one of HANDLED, UNHANDLED, TRANSITION, or SUPER.
        // UNHANDLED is usually returned on a branching handler that
        // might also return HANDLED. A call to SUPER should be made
        // to figure out where the event should bubble up to.
        // (The primary purpose of the UNHANDLED outcome is to facilitate
        // authoring of sophisticated guard logic within a switch case
        // without having to repeat the TW_SUPER(). This is so that the
        // TW_SUPER() is written in one place in the state method, which
        // is less error-prone should it need to be updated.)
        m_stateTemp = m_stateCurrent;
        do {
            source = m_stateTemp;
            out = (this->*m_stateTemp)(event);
            if (out == DISPATCH_UNHANDLED) {
                out = _TW_PSEUDO(m_stateTemp, SIG_SUPER);
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
            if (DISPATCH_HANDLED == _TW_PSEUDO(m_stateTemp, SIG_LEAVE)) {
                _TW_PSEUDO(m_stateTemp, SIG_SUPER);
            }
        }
        m_stateCurrent = source;

        // transition to self
        if (source == target) {
            // All we need to do is leave-and-enter the state.
            _TW_PSEUDO(source, SIG_LEAVE);
            _TW_PSEUDO(target, SIG_ENTER);
            m_stateCurrent = target;
        }
        else {
            // fill path with list of target supers (including target)
            p = 0;
            m_stateTemp = target;
            do {
                path[p] = m_stateTemp;
                out = _TW_PSEUDO(m_stateTemp, SIG_SUPER);
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
                    if (DISPATCH_HANDLED == _TW_PSEUDO(m_stateCurrent, SIG_LEAVE)) {
                        _TW_PSEUDO(m_stateCurrent, SIG_SUPER);
                    }
                    m_stateCurrent = m_stateTemp;
                }
            } while (m_stateCurrent);

            // drill down into the target
            if (-1 != enter_start) {
                for (p = enter_start; p >= 0; p--) {
                    _TW_PSEUDO(path[p], SIG_ENTER);
                    m_stateCurrent = path[p];
                }
            }
        } // not a self-transition

        // Handle the initial transition(s) of the target state
        while (DISPATCH_TRANSITION == _TW_PSEUDO(m_stateCurrent, SIG_INIT)) {
            source = m_stateCurrent;    // the transition "from" state
            target = m_stateTemp;       // the transition "to" state
            p = 0;
            for (p = 0; m_stateTemp && (m_stateTemp != source); p++) {
                path[p] = m_stateTemp;
                _TW_PSEUDO(m_stateTemp, SIG_SUPER);
            }
            for (; p > 0; p--) {
                _TW_PSEUDO(path[p-1], SIG_ENTER);
            }
            m_stateCurrent = target;
        }
    }


    void
    HSM::dispatchIdle() {
        this->dispatch(&(PSEUDOEVENTS[SIG_IDLE]));
    }


    void
    HSM::dispatchAll(IQueue* queue, bool idleIfEmpty) {
        if (queue->size()) {
            while (queue->size()) {
                this->dispatch(queue->front());
                queue->pop_front();
            }
        }
        else {
            if (idleIfEmpty) {
                this->dispatchIdle();
            }
        }
    }


    DispatchOutcome
    HSM::stateROOT(const Event* event) {
        if (SIG_SUPER == event->signal) {
            return TW_SUPER(0);
        }
        return TW_HANDLED();
    }



    //----------------------------------------------------------------------
    // BACKWARDS-COMPATIBILITY FUNCTIONS
    //

    void
    dispatchIdle(FSM* machine) {
        machine->dispatchIdle();
    }


    void
    dispatchIdle(HSM* machine) {
        machine->dispatchIdle();
    }


    void
    dispatchAll(FSM* machine, IQueue* queue, bool idleIfEmpty) {
        machine->dispatchAll(queue, idleIfEmpty);
    }


    void
    dispatchAll(HSM* machine, IQueue* queue, bool idleIfEmpty) {
        machine->dispatchAll(queue, idleIfEmpty);
    }



};


