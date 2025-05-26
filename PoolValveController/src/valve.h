#ifndef Valve_h
#define Valve_h

#include "ValveConfig.h"
#include "Arduino.h"
#include <RelayManager.h>
#include <OnOffDetector.h>
#include <Console.h>
#include <NuvIoTState.h>
#include <WebSocketsServer.h>

class Valve
{
private:
    unsigned long m_finishCount;
    unsigned long m_nextTimeout;

    bool m_isMoving = false;

    RelayManager *m_relayMgr;
    WebSocketsServer *m_webSocket;
    Console *m_console;
    NuvIoTState *m_state;
    uint8_t m_index;
   
private:
    currentPosition_t m_nextPosition;


    typedef enum moveDirection{
        moveDirection_off,
        moveDirection_cw,
        moveDirection_ccw,
    } moveDirection_t;

    void setValve(moveDirection_t angle, long duration){
        switch (angle){
            case moveDirection_off:
                m_console->print("OFF ");
                m_webSocket->broadcastTXT("{\"messageId\":\"valveState\",\"valve\":\"" + m_config->Key + "\",\"state\":\"off\"}");
                break;
            case moveDirection_cw:
                m_console->print("CW ");
                m_webSocket->broadcastTXT("{\"messageId\":\"valveState\",\"valve\":\"" + m_config->Key + "\",\"state\":\"cw\",\"duration\":" + String(duration) + "}");
                break;
            case moveDirection_ccw:
                m_console->print("CCW ");
                m_webSocket->broadcastTXT("{\"messageId\":\"valveState\",\"valve\":\"" + m_config->Key + "\",\"state\":\"ccw\",\"duration\":" + String(duration) + "}");
                break;
        }

        m_relayMgr->setRelay(m_config->PowerPin, angle != moveDirection_off ? true : false);
        m_relayMgr->setRelay(m_config->DirPin, angle == moveDirection_cw ? true : false);

        if (duration > 0){
            m_nextTimeout = duration + millis();
            m_console->println("Timeout: " + String(duration) + " " + String(m_nextTimeout));
            m_isMoving = true;
        }
        else{
            m_isMoving = false;
            m_console->println("Timeout: 0");
        }
    }

public:
        ValvesConfig *m_configs;
        ValveConfig *m_config;

    void init(Console *console, RelayManager *mgr, WebSocketsServer *webSocket, ValvesConfig *valvesConfig, int idx){
        m_nextPosition = currentPosition_unknown;

        m_configs = valvesConfig;
        m_relayMgr = mgr;
        m_console = console;
        m_webSocket = webSocket;
   
        m_index = idx;
        m_config = &m_configs->getValves()[idx];

        m_nextTimeout = 0;
    }

    String Name;

    String getKey() {
        return m_config->Key;
    }

    String getName(){
        return m_config->Name;
    }

  
    String getStatusName(){
        if (m_isMoving){
            return "Moving";
        }
        else{
            switch (m_config->CurrentState){
                case currentPosition_unknown: return "Unknown";
                case currentPosition_0: return m_config->Pos0Name;
                case currentPosition_90: return m_config->Pos90Name;
                case currentPosition_180: return m_config->Pos180Name;
            }
        }

        return "?";
    }

    String getStatusKey(){
        if (m_isMoving){
            return "moving";
        }
        else{
            switch (m_config->CurrentState){
                case currentPosition_unknown: return "unknown";
                case currentPosition_0: return m_config->Pos0Key;
                case currentPosition_90: return m_config->Pos90Key;
                case currentPosition_180: return m_config->Pos180Key;
            }   
        }

        return "?";
    }


    currentPosition_t getCurrentPosition() { return m_config->CurrentState; }

    void reset() {
        currentPosition_t lastPosition = m_config->CurrentState;
        m_config->CurrentState = currentPosition_unknown;
        setCurrentPosition(lastPosition);
    }

    void calibrate() {
        setValve(moveDirection_ccw, m_config->Timing * 2);
        m_nextPosition = currentPosition_0;
    }

    void setCurrentPosition(String nextPosition){
        if (nextPosition == m_config->Pos0Key)
        {
            setCurrentPosition(currentPosition_0);
        }
        else if (nextPosition == m_config->Pos90Key)
        {
            setCurrentPosition(currentPosition_90);
        }
        else if (nextPosition == m_config->Pos180Key)
        {
            setCurrentPosition(currentPosition_180);
        }
        else {
            m_console->printError("Unknown position: [>" + nextPosition + "<] [" + m_config->Pos90Key + "]");
            return;
        }
    }

    void setCurrentPosition(currentPosition_t nextPosition) {
        if (m_isMoving)
            return;

        if (nextPosition != m_config->CurrentState)
        {
            m_nextPosition = nextPosition;

            switch (m_config->CurrentState)
            {
            case currentPosition_unknown:
                setValve(moveDirection_ccw, m_config->Timing * 2);
                m_console->println("position was unknown, moving to start.");
                break;
            case currentPosition_0:
                switch (nextPosition){
                case currentPosition_90:
                    setValve(moveDirection_cw, m_config->Timing);
                     m_console->println("position was 0, moving to middle.");
                    break;
                case currentPosition_180:
                    setValve(moveDirection_cw, m_config->Timing * 2);
                     m_console->println("position was 0, moving to end.");
                    break;
                default:
                    break;
                }
                break;
            case currentPosition_90:
                switch (nextPosition){
                case currentPosition_0:
                    setValve(moveDirection_ccw, m_config->Timing);
                    m_console->println("position was 90, moving to start.");
                    break;
                case currentPosition_180:
                    setValve(moveDirection_cw, m_config->Timing);
                    m_console->println("position was 90, moving to end.");
                    break;
                default:
                    break;
                }

                break;
            case currentPosition_180:
                switch (nextPosition){
                case currentPosition_0:
                    m_console->println("position was 180, moving to middle.");
                    setValve(moveDirection_ccw, m_config->Timing * 2);
                    break;
                case currentPosition_90:
                    m_console->println("position was 180, moving to start * 2.");
                    setValve(moveDirection_ccw, m_config->Timing);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
    }

    bool getIsMoving(){
        return m_isMoving;
    }

    void Update(){
        if (millis() > m_nextTimeout && m_isMoving){
            m_isMoving = false;
            if (m_config->CurrentState == currentPosition_unknown){
                m_config->CurrentState = currentPosition_0;
                m_console->println("position was unknown, moved to start.");

                if(m_nextPosition == currentPosition_0){
                    setValve(moveDirection_off, 0);
                    m_configs->write();
                }
                else{
                    setCurrentPosition(m_nextPosition);
                }
            }
            else{
                m_isMoving = false;
                m_config->CurrentState = m_nextPosition;
                setValve(moveDirection_off, 0);
                m_console->println("all done moved to " + getStatusName());
                m_configs->write();
            
            }
        }
    }
};

#endif
