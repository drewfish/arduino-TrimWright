#include <Arduino.h>
#include "TrimWright.h"
using namespace TrimWright;



class PinDigitalInput {
    private:
        static const uint8_t    E_LOW   = 0;
        static const uint8_t    E_HIGH  = 1;
        static const uint8_t    E_CLICK = 2;
        static const uint8_t    E_HOLD  = 3;
        int8_t      cfg_pin;
        Signal      cfg_events;
        uint32_t    cfg_bounce;
        uint32_t    cfg_hold;
        bool        m_high;
        bool        m_held;     // reported a HOLD and waiting for a HIGH
        uint32_t    to_bounce;
        uint32_t    to_hold;


        void pub(uint32_t now, IQueue* q) {
            Event e;
            if (m_high) {
                e.signal = cfg_events + E_HIGH;
                q->push_back(&e);
                if (m_held) {
                    m_held = false;
                }
                else {
                    e.signal = cfg_events + E_CLICK;
                    q->push_back(&e);
                    if (cfg_hold) {
                        to_hold = 0;
                    }
                }
            }
            else {
                e.signal = cfg_events + E_LOW;
                q->push_back(&e);
                if (cfg_hold) {
                    to_hold = now + cfg_hold;
                }
            }
        }


    public:
        PinDigitalInput(
            int8_t      pin,
            Signal      eventOffset,
            uint32_t    timeoutBounce,
            uint32_t    timeoutHold
        ) :
            cfg_pin(pin),
            cfg_events(eventOffset),
            cfg_bounce(timeoutBounce),
            cfg_hold(timeoutHold),
            m_high(false),
            m_held(false),
            to_bounce(0),
            to_hold(0)
        {
            // nothing else to do
        }


        void init() {
            pinMode(cfg_pin, INPUT_PULLUP);
            m_high = digitalRead(cfg_pin) == HIGH;
            Serial.print("cfg_bounce ");
            Serial.println(cfg_bounce);
        }


        void poll(uint32_t now, IQueue* q) {
            bool nowHigh = digitalRead(cfg_pin) == HIGH;

            if (to_bounce) {
                if (now >= to_bounce) {
                    if (m_high != nowHigh) {
                        m_high = nowHigh;
                        pub(now, q);
                    }
                    to_bounce = 0;
                }
                return;
            }

            if (to_hold) {
                if (now >= to_hold) {
                    Event e;
                    e.signal = cfg_events + E_HOLD;
                    q->push_back(&e);
                    m_held = true;
                    to_hold = 0;
                }
            }

            if (m_high == nowHigh) {
                return;
            }

            if (cfg_bounce) {
                // bounced off start:  start timer
                to_bounce = now + cfg_bounce;
            }
            else {
                m_high = nowHigh;
                pub(now, q);
            }
        }


        bool isHigh() {
            return m_high;
        }
};



class PinDigitalOutput {
    private:
        int8_t      cfg_pin;
        bool        m_high;
        uint32_t    cfg_cycle;
        uint32_t    to_cycle;

    public:
        PinDigitalOutput(int8_t pin) :
            cfg_pin(pin),
            m_high(true),
            cfg_cycle(0),
            to_cycle(0)
        {
            // nothing else to do
        }

        
        void init() {
            pinMode(cfg_pin, OUTPUT);
            digitalWrite(cfg_pin, m_high ? HIGH : LOW);
        }


        void poll(uint32_t now, IQueue* q) {
            if (to_cycle && (now >= to_cycle)) {
                m_high = !m_high;
                digitalWrite(cfg_pin, m_high ? HIGH : LOW);
                to_cycle = now + cfg_cycle;
            }
        }


        void setHigh(bool high) {
            m_high = high;
            cfg_cycle = 0;
            to_cycle = 0;
            digitalWrite(cfg_pin, m_high ? HIGH : LOW);
        }


        bool isHigh() {
            return m_high;
        }

        
        void blink(uint32_t cycle) {
            cfg_cycle = cycle;
            to_cycle = millis() + cfg_cycle;
        }
};


enum {
    SIG_SWITCH_LOW = SIG_USER,
    SIG_SWITCH_HIGH,
    SIG_SWITCH_CLICK,
    SIG_SWITCH_HOLD
};


QueueRingBuffer<Event, 6>   queue;
PinDigitalInput             swtch(16, Signal(SIG_SWITCH_LOW), 20, 1000);
PinDigitalOutput            led(9);
uint32_t                    lastDump;


class App : public FSM {
    public:
        DispatchOutcome stateON(Event* e) {
            switch(e->signal) {
                case SIG_ENTER:
                    Serial.println(F("========================================= <enter> ON"));
                    led.setHigh(true);
                    return TW_HANDLED();
                case SIG_SWITCH_CLICK:
                    Serial.println(F("<click>"));
                    return TW_TRANSITION(&App::stateOFF);
                case SIG_SWITCH_HOLD:
                    Serial.println(F("<hold>"));
                    return TW_TRANSITION(&App::stateFAST);
                default:
                    return TW_HANDLED();
            }
        }
        DispatchOutcome stateOFF(Event* e) {
            switch(e->signal) {
                case SIG_ENTER:
                    Serial.println(F("========================================= <enter> OFF"));
                    led.setHigh(false);
                    return TW_HANDLED();
                case SIG_SWITCH_CLICK:
                    Serial.println(F("<click>"));
                    return TW_TRANSITION(&App::stateON);
                case SIG_SWITCH_HOLD:
                    Serial.println(F("<hold>"));
                    return TW_TRANSITION(&App::stateFAST);
                default:
                    return TW_HANDLED();
            }
        }
        DispatchOutcome stateSLOW(Event* e) {
            switch(e->signal) {
                case SIG_ENTER:
                    Serial.println(F("========================================= <enter> SLOW"));
                    led.setHigh(! led.isHigh());
                    led.blink(1000);
                    return TW_HANDLED();
                case SIG_SWITCH_CLICK:
                    Serial.println(F("<click>"));
                    return TW_TRANSITION(&App::stateFAST);
                case SIG_SWITCH_HOLD:
                    Serial.println(F("<hold>"));
                    if (led.isHigh()) {
                        return TW_TRANSITION(&App::stateOFF);
                    } else {
                        return TW_TRANSITION(&App::stateON);
                    }
                default:
                    return TW_HANDLED();
            }
        }
        DispatchOutcome stateFAST(Event* e) {
            switch(e->signal) {
                case SIG_ENTER:
                    Serial.println(F("========================================= <enter> FAST"));
                    led.setHigh(! led.isHigh());
                    led.blink(300);
                    return TW_HANDLED();
                case SIG_SWITCH_CLICK:
                    Serial.println(F("<click>"));
                    return TW_TRANSITION(&App::stateSLOW);
                case SIG_SWITCH_HOLD:
                    Serial.println(F("<hold>"));
                    if (led.isHigh()) {
                        return TW_TRANSITION(&App::stateOFF);
                    } else {
                        return TW_TRANSITION(&App::stateON);
                    }
                default:
                    return TW_HANDLED();
            }
        }
} app;


void setup() {
    Serial.begin(57600);
    Serial.println(F("----------------BOOTED----------------"));
    swtch.init();
    led.init();
    led.setHigh(true);
    app.init((State) &App::stateON);
    lastDump = millis();
}


void loop() {
    uint32_t now = millis();
    swtch.poll(now, &queue);
    led.poll(now, &queue);

    dispatchAll(&app, &queue, false);
}


