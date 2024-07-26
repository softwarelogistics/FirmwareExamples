#ifndef Valve_h
#define Valve_h

#include "Arduino.h"
#include <RelayManager.h>
#include <OnOffDetector.h>
#include <Console.h>
#include <NuvIoTState.h>

typedef enum currentPosition
{
    currentPosition_unknown,
    currentPosition_0,
    currentPosition_90,
    currentPosition_180
} currentPosition_t;

class Valve
{
private:
    int m_powerPin;
    int m_dirPin;
    int m_centerIndex;
    unsigned long m_finishCount;
    unsigned long m_nextTimeout;
    String m_pos0Name;
    String m_pos90Name;
    String m_pos180Name;

    int m_move90_ms = 0;
    bool m_isMoving = false;

    RelayManager *m_relayMgr;
    OnOffDetector *m_onOffDetector;
    Console *m_console;
    NuvIoTState *m_state;
    String m_name;

private:
    currentPosition_t m_currentPosition;
    currentPosition_t m_nextPosition;

    typedef enum moveDirection
    {
        moveDirection_off,
        moveDirection_cw,
        moveDirection_ccw,
    } moveDirection_t;

    void setValve(moveDirection_t angle, long duration)
    {
        switch (angle)
        {
        case moveDirection_off:
            m_console->print("OFF ");
            break;
        case moveDirection_cw:
            m_console->print("CW ");
            break;
        case moveDirection_ccw:
            m_console->print("CCW ");
            break;
        }

        m_relayMgr->setRelay(m_powerPin, angle != moveDirection_off ? true : false);
        m_relayMgr->setRelay(m_dirPin, angle == moveDirection_cw ? true : false);

        if (duration > 0)
        {
            m_nextTimeout = duration + millis();
            m_console->println("Timeout: " + String(duration) + " " + String(m_nextTimeout));
            m_isMoving = true;
        }
        else
        {
            m_isMoving = false;
            m_console->println("Timeout: 0");
        }
    }

public:
    Valve(Console *console, RelayManager *mgr, NuvIoTState *state, OnOffDetector *detector, String name, int powerPin, int dirPin, int centerIndex, String pos0Name, String pos90Name, String pos180Name)
    {
        m_pos0Name = pos0Name;
        m_pos90Name = pos90Name;
        m_pos180Name = pos180Name;

        m_name = name;

        m_currentPosition = currentPosition_unknown;
        m_nextPosition = currentPosition_unknown;

        m_state = state;
        m_relayMgr = mgr;
        m_onOffDetector = detector;
        m_console = console;

        m_nextTimeout = 0;

        m_powerPin = powerPin;
        m_dirPin = dirPin;
        m_centerIndex = centerIndex;

    }

    void init() {
        m_state->registerInt((m_name + "state").c_str(), currentPosition_unknown);
        m_state->registerInt((m_name + "ms").c_str(), 20000);

        m_currentPosition = (currentPosition_t)m_state->getInt((m_name + "state").c_str());
        m_move90_ms = m_state->getInt((m_name + "ms").c_str());
    }

    void setTiming(uint16_t timing)
    {
        m_move90_ms = timing;
        m_state->registerInt((m_name + "ms").c_str(), 20000);
    }

    uint16_t getTiming()
    {
        return m_move90_ms;
    }

    String getStatus()
    {
        if (m_isMoving)
        {
            return "moving";
        }
        else
        {
            switch (m_currentPosition)
            {
            case currentPosition_unknown:
                return "unknown";
                break;
            case currentPosition_0:
                return m_pos0Name;
                break;
            case currentPosition_90:
                return m_pos90Name;
                break;
            case currentPosition_180:
                return m_pos180Name;
                break;
            }
        }
    }

    currentPosition_t getCurrentPosition() { return m_currentPosition; }

    void reset() {
        currentPosition_t lastPosition = m_currentPosition;
        m_currentPosition = currentPosition_unknown;
        setCurrentPosition(lastPosition);
    }

    void calibrate() {
        setValve(moveDirection_ccw, m_move90_ms * 2);
        m_nextPosition = currentPosition_0;
    }

    void setCurrentPosition(currentPosition_t nextPosition)
    {
        if (m_isMoving)
            return;

        if (nextPosition != m_currentPosition)
        {
            m_nextPosition = nextPosition;

            switch (m_currentPosition)
            {
            case currentPosition_unknown:
                setValve(moveDirection_ccw, m_move90_ms * 2);
                break;
            case currentPosition_0:
                switch (nextPosition)
                {
                case currentPosition_90:
                    setValve(moveDirection_cw, m_move90_ms);
                    break;
                case currentPosition_180:
                    setValve(moveDirection_cw, m_move90_ms * 2);
                    break;
                default:
                    break;
                }
                break;
            case currentPosition_90:
                switch (nextPosition)
                {
                case currentPosition_0:
                    setValve(moveDirection_ccw, m_move90_ms);
                    break;
                case currentPosition_180:
                    setValve(moveDirection_cw, m_move90_ms);
                    break;
                default:
                    break;
                }

                break;
            case currentPosition_180:
                switch (nextPosition)
                {
                case currentPosition_0:
                    setValve(moveDirection_ccw, m_move90_ms * 2);
                    break;
                case currentPosition_90:
                    setValve(moveDirection_ccw, m_move90_ms);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        m_state->setInt((m_name + "state").c_str(), (int32_t)nextPosition);
    }

    bool getIsMoving()
    {
        return m_isMoving;
    }

    void Update()
    {
        if (millis() > m_nextTimeout && m_isMoving)
        {
            m_isMoving = false;
            if (m_currentPosition == currentPosition_unknown)
            {
                m_currentPosition = currentPosition_0;
                m_console->println("position was unknown, moved to start.");

                setCurrentPosition(m_nextPosition);
            }
            else
            {
                m_isMoving = false;
                m_currentPosition = m_nextPosition;
                setValve(moveDirection_off, 0);
                m_console->println("all done moved to " + getStatus());
            }
        }
    }
};

#endif
