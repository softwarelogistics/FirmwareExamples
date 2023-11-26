#define RELAY_BRD_V1

#include <Arduino.h>
#include <StringSplitter.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>

#include <NuvIoT.h>
#include <MedianFilter.h>

#include <uri/UriRegex.h>

#define FIRMWARE_VERSION "2.6.5"
#define HARDWARE_REVISION "5"
#define FW_SKU "POOL001"

#include <PageHandler.h>
#include "pages.h"

boolean _shouldHeat = false;
boolean _heaterOn = false;


String _lastError = "none";
double temperatureIn = -1;
double temperatureOut = -1;
bool hasFlow = false;
bool lowPressure = false;
bool highPressure = false;

int poolMode;
float spaSetPoint;
float poolSetPoint;

WebServer *webServer = new WebServer(80);

void redirectToSensorPage()
{
    webServer->sendHeader("Location", String("/sensor"), true);
    webServer->send(302, "text/plain");
}

void setHeater(boolean isOn)
{
    _heaterOn = isOn;
    state.updateProperty("TrueFalse", "heateron", isOn ? "true" : "false");
}

void handleTopic(String topic)
{
}

void handleApi(String noun, String verb)
{
    redirectToSensorPage();
    setHeater(verb == "on");
}

class PoolPageHandler : public RequestHandler
{
    bool canHandle(HTTPMethod method, String uri)
    {
        return true;
    }

    bool handle(WebServer &server, HTTPMethod requestMethod, String requestUri)
    {
        StringSplitter *splitter = new StringSplitter(requestUri, '/', 6);
        uint8_t itemCount = splitter->getItemCount();

        for (int i = 0; i < itemCount; i++)
        {
            String item = splitter->getItemAtIndex(i);
            console.println(String(i + 1) + ". " + item);
        }

        if (itemCount > 2)
        {
            if (splitter->getItemAtIndex(0) == "api" || splitter->getItemAtIndex(0) == "set")
            {
                if (splitter->getItemAtIndex(1) == "heater")
                {
                    setHeater(splitter->getItemAtIndex(2) == "on");
                }
            }
        }

        delete splitter;

        sendSensorData(_heaterOn, _shouldHeat, &ioValues, FIRMWARE_VERSION);
        return true;
    }
} pageHandler;

#define HEATER_MODE_OFF 0
#define HEATER_MODE_POOL 1
#define HEATER_MODE_HOT_TUB 2

void setup()
{
    configureFileSystem();

    state.init(FW_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);

    configureConsole();

    console.setVerboseLogging(false);

    initPins();

    writeConfigPins();

    ioConfig.load();
    sysConfig.load();
    sysConfig.SendUpdateRateMS = 5000;

    ioConfig.GPIO1Config = GPIO_CONFIG_DBS18;
    ioConfig.GPIO1Name = "temperatureIn";

    ioConfig.GPIO2Config = GPIO_CONFIG_DBS18;
    ioConfig.GPIO2Name = "temperatureOut";

    ioConfig.GPIO6Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO6Name = "lowpressure";

    ioConfig.GPIO7Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO7Name = "flow";

    ioConfig.GPIO8Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO8Name = "highpressure";

    ioConfig.Relay1Name = "heat1";
    ioConfig.Relay1Enabled = true;
    ioConfig.Relay2Name = "heat2";
    ioConfig.Relay2Enabled = true;
    ioConfig.Relay3Name = "fan";
    ioConfig.Relay3Enabled = true;

    welcome(FW_SKU, FIRMWARE_VERSION);

    String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Pool Manager" : sysConfig.DeviceId);
    BT.begin(btName.c_str(), FW_SKU);

    ledManager.setup(&ioConfig);
    ledManager.setOnlineFlashRate(1);
    ledManager.setErrFlashRate(0);

    configureI2C();

    probes.setup(&ioConfig);
    onOffDetector.setup(&ioConfig);
    relayManager.setup(&ioConfig);

    state.registerBool("heateron", false);

    state.registerInt("poolmode", HEATER_MODE_OFF);
    state.registerFloat("poolsetpoint", 89);
    state.registerFloat("hottubsetpoint", 103);

    poolMode = state.getInt("poolmode");
    spaSetPoint = state.getFlt("hottubsetpoint");
    poolSetPoint = state.getFlt("poolsetpoint");

    wifiMgr.setup();
}

int idx = 0;

long nextSend = 0;

boolean webServerSetup = false;

void setupWebServer()
{
    webServer->on("/heater/on", HTTP_GET, []()
                  { handleApi("heater", "on"); });
    webServer->on("/heater/off", HTTP_GET, []()
                  { handleApi("heater", "off"); });

    webServer->addHandler(&pageHandler);
    webServer->begin();

    webServerSetup = true;
}

void readSensors() {
    temperatureIn = ioValues.getIODoubleValue(0);
    temperatureOut = ioValues.getIODoubleValue(1);
    hasFlow = ioValues.getIOValue(5) == "0";
    lowPressure = ioValues.getIOValue(6) == "1";
    highPressure = ioValues.getIOValue(7) == "1";
}

void engageHeater() {
    poolMode = state.getInt("poolmode");
    spaSetPoint = state.getFlt("hottubsetpoint");
    poolSetPoint = state.getFlt("poolsetpoint");

    _lastError = "none";

    if(!hasFlow) {
        _shouldHeat = false;
        _lastError = "lowpressure";
    }
    else if(poolMode == HEATER_MODE_OFF){
        _shouldHeat = false;
    }
    else {        
        float setPoint = poolMode == HEATER_MODE_POOL ? poolSetPoint : spaSetPoint;
        if(temperatureIn > setPoint + 0.5) {
            _shouldHeat = false;
        }
        else if(temperatureIn < setPoint - 0.5) {
            _shouldHeat = true;
        }
    }
    
    relayManager.setRelay(0, _shouldHeat);
    relayManager.setRelay(1, _shouldHeat);
    relayManager.setRelay(2, _shouldHeat);
}

void sendUpdate() {
    String msg = "{'heating':" + String(_shouldHeat) +
                    ",'poolMode':" + String(poolMode) +
                    ",'spaSetPoint':" + String(spaSetPoint) +
                    ",'poolSetPoint':" + String(poolSetPoint) +
                    ",'in':" + String(temperatureIn) +
                    ",'out':" + String(temperatureOut) +
                    ",'lowPressure':" + (lowPressure ? "'ok'" : "'warning'") +
                    ",'flow':" + (hasFlow ? "'ok'" : "'warning'") +
                    ",'highpressure':" + (highPressure ? "'ok'" : "'warning'") +
                    ",'ipaddress':'" + wifiMgr.getIPAddress() + "'" +
                    ",'err':'" + _lastError + "'}";
    console.println(msg);

    if (wifiMgr.isConnected())
    {
        wifiMQTT.publish("poolctrlr/poolheater/" + sysConfig.DeviceId, msg);
        wifiMQTT.sendIOValues(&ioValues);
        wifiMQTT.sendRelayStatus(&relayManager);
    }
}

void loop()
{
    commonLoop();
    readSensors();
    engageHeater();

    if (wifiMgr.isConnected())
    {
        if (!webServerSetup)
        {
            setupWebServer();
        }

        webServer->handleClient();
    }

    if (nextSend < millis() || nextSend == 0)
    {
        nextSend = millis() + sysConfig.SendUpdateRateMS;        

        sendUpdate();
    }
}