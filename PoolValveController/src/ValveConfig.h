#ifndef VALVE_CONFIG_H
#define VALVE_CONFIG_H

#include "Arduino.h"

#include <ArduinoJson.h>

#include <SPIFFS.h>
#include <Console.h>

#define SYSCONFIG_FN "/valveconfig.json"


typedef enum currentPosition{
    currentPosition_unknown = -1,
    currentPosition_0 = 0,
    currentPosition_90 = 90,
    currentPosition_180 = 180
} currentPosition_t;


class ValveMode 
{
    public:
    String Name;
    String Positions[];
};

class ValveConfig
{
    public:
    String Name;
    String Key;

    String Pos0Name;
    String Pos0Key;

    String Pos90Name;
    String Pos90Key;

    String Pos180Name;
    String Pos180Key;

    uint8_t PowerPin;
    uint8_t DirPin;

    uint16_t Timing = 20000; // Default timing to 20 seconds
    currentPosition_t CurrentState;
    
    void addToparent(JsonArray parent){
        JsonDocument doc;
        doc["name"] = Name;
        doc["key"] = Key;
        doc["pos0Name"] = Pos0Name;
        doc["pos0Key"] = Pos0Key;
        doc["pos90Name"] = Pos90Name;
        doc["pos90Key"] = Pos90Key;
        doc["pos180Name"] = Pos180Name;
        doc["pos180Key"] = Pos180Key;
        doc["powerPin"] = PowerPin;
        doc["dirPin"] = DirPin;
        doc["timing"] = Timing; 
        doc["currentState"] = CurrentState;
        parent.add(doc);
    }

    void parseFromJson(JsonObject obj){
        Name = obj["name"].as<String>();
        Key = obj["key"].as<String>();
        Pos0Name = obj["pos0Name"].as<String>();
        Pos0Key = obj["pos0Key"].as<String>();
        Pos90Name = obj["pos90Name"].as<String>();
        Pos90Key = obj["pos90Key"].as<String>();
        Pos180Name = obj["pos180Name"].as<String>();
        Pos180Key = obj["pos180Key"].as<String>();
        PowerPin = obj["powerPin"] | (uint8_t)0;
        DirPin = obj["dirPin"] | (uint8_t)0;
        Timing = obj["timing"] | (uint16_t)20000; // Default timing to 20 seconds if not specified
        CurrentState = obj["currentState"] | (currentPosition_t)-1; // Default to 0 if not specified
    }

    void writeField(String field, String value) {
        if (field == "name") {
            Name = value;
        } else if (field == "key") {
            Key = value;
        } else if (field == "pos0Name") {
            Pos0Name = value;
        } else if (field == "pos0Key") {
            Pos0Key = value;
        } else if (field == "pos90Name") {
            Pos90Name = value;
        } else if (field == "pos90Key") {
            Pos90Key = value;
        } else if (field == "pos180Name") {
            Pos180Name = value;
        } else if (field == "pos180Key") {
            Pos180Key = value;
        } else if (field == "powerPin") {
            PowerPin = value.toInt();
        } else if (field == "dirPin") {
            DirPin = value.toInt();
        } else if (field == "timing") {
            Timing = value.toInt();
        }
    }
};


class ValvesConfig
{
private:
    Console *m_pConsole;
    int m_valveCount;
    public:

    ValveConfig *m_valves; // Assuming 3 valves, adjust as needed
    ValveMode *m_modes; // Assuming multiple modes, adjust as needed

    ValvesConfig(Console *pConsole, int valveCount)
    {
        m_valveCount = valveCount;
        m_pConsole = pConsole;
        m_valves = new ValveConfig[valveCount]; 
    }

public:    
    void load()
    {
        File file = SPIFFS.open(SYSCONFIG_FN, FILE_READ);
        if (file)
        {
            m_pConsole->printVerbose("valveconfig=fileexits;");
            String json = file.readString();
            file.close();

            if (json.length() == 0)
            {
                m_pConsole->printVerbose("valveconfig=errorread; // file length = 0");
                file.close();
                setDefaults();

                write();
            }
            else
            {
                if (!parseJSON(json))
                {
                    m_pConsole->printVerbose("valveconfig=errorread; //could not parse json");

                    setDefaults();
                    write();
                }
                else
                {
                    m_pConsole->printVerbose("valveconfig=fileread;");
                }
            }
        }
        else
        {
            m_pConsole->printVerbose("valveconfig=filedoesnotexists;");
            setDefaults();
            write();
        }
    }

    ValveConfig* getValve(String valveKey){
        for(uint8_t i = 0; i < m_valveCount; i++)
        {
            if(m_valves[i].Key == valveKey)
            {
                return &m_valves[i];
            }
        }

        return NULL;
    }

    ValveConfig *getValves() {
        return m_valves;
    }

    void write()
    {
        File file = SPIFFS.open(SYSCONFIG_FN, FILE_WRITE);
        if (!file)
        {
            m_pConsole->printError("sysconfig=failwrite; // could not open file or write");
        }
        else
        {
            String json = toJSON();
            size_t written = file.print(json);
            file.flush();
            file.close();

            if (written != json.length())
            {
                m_pConsole->printError("sysconfig=failwrite; // mismatch write, written: " + String(written) + " size: " + String(json.length()));
            }
            else
            {
                m_pConsole->println("sysconfig=writefile; // wrote " + String(written) + " bytes to " + SYSCONFIG_FN);
            }
        }
    }

    
    String toJSON(){
        JsonDocument doc;

        // Add your JSON serialization logic here
        // Example: doc["name"] = Name;
        // Add your serialization logic here
        JsonArray valveArray = doc["valves"].to<JsonArray>();
        for (int i = 0; i < m_valveCount; i++)
        {            
            m_valves[i].addToparent(valveArray);
        }

        String json;
        serializeJson(doc, json);
        return json;
    }


    bool parseJSON(String json)
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        JsonArray array= doc["valves"].as<JsonArray>();

        for(int idx = 0; idx < m_valveCount && idx < array.size(); idx++)
        {
            m_valves[idx].parseFromJson(array[idx].as<JsonObject>());
        }

        return true;
    }
    
    void setDefaults(){
        m_valves[0].Name = "Jets";
        m_valves[0].Key = "jets";
        m_valves[0].Pos0Name = "Jets Off";
        m_valves[0].Pos0Key = "jets_off";
        m_valves[0].Pos90Name = "Jets 50%";   
        m_valves[0].Pos90Key = "jets_50";
        m_valves[0].Pos180Name = "Jets 100%";
        m_valves[0].Pos180Key = "jets_100";
        m_valves[0].PowerPin = 0;
        m_valves[0].DirPin = 1;
        m_valves[0].Timing = 20000; // Default timing to 20 seconds
        m_valves[0].CurrentState = currentPosition_unknown; // Default to -1 if not specified

        m_valves[1].Name = "Source";
        m_valves[1].Key = "source";
        m_valves[1].Pos0Name = "Pool";
        m_valves[1].Pos0Key = "pool";
        m_valves[1].Pos90Name = "Pool and Spa";   
        m_valves[1].Pos90Key = "both";
        m_valves[1].Pos180Name = "Spa";
        m_valves[1].Pos180Key = "spa";
        m_valves[1].PowerPin = 2;
        m_valves[1].DirPin = 3;
        m_valves[1].Timing = 20000; // Default timing to 20 seconds
        m_valves[1].CurrentState = currentPosition_unknown; // Default to -1 if not specified
    
        m_valves[2].Name = "Output";
        m_valves[2].Key = "output";
        m_valves[2].Pos0Name = "Pool";
        m_valves[2].Pos0Key = "pool";
        m_valves[2].Pos90Name = "Pool and Spa";   
        m_valves[2].Pos90Key = "both";
        m_valves[2].Pos180Name = "Spa";
        m_valves[2].Pos180Key = "spa";
        m_valves[2].PowerPin = 4;
        m_valves[2].DirPin = 5;  
        m_valves[2].Timing = 20000; // Default timing to 20 seconds
        m_valves[2].CurrentState = currentPosition_unknown; // Default to -1 if not specified
    }
};



#endif