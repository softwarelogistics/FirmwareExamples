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

#define FIRMWARE_VERSION "2.5.2"
#define HARDWARE_REVISION "5"
#define FW_SKU "POOL001"

#include <PageHandler.h>
#include "pages.h"

/*
 * Temperature 2.62 - 79.7 (in)
 * Temperature 2.68 - 82.7
 * Temperature 2.53 =  84.0
 * Temperature 2.59 =  85.5
 * Temperature 2.74 =  88.3
 * Temperature 2.87 =  92.2
 * Temperature 2.86 =  94
 * Temperature 2.89  = 98
 * Temperature 2.95  = 98
 *  2.90 - 90
 *             2.97 =  101.5
 *
 */

boolean _shouldHeat = false;
boolean _heaterOn = false;

WebServer *webServer = new WebServer(80);


void redirectToSensorPage()
{
    webServer->sendHeader("Location", String("/sensor"), true);
    webServer->send(302, "text/plain");
}

void setHeater(boolean isOn){
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


void setup()
{
    delay(5000);

    configureFileSystem();

    state.init(FW_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);

    configureConsole();

    initPins();

    ioConfig.load();
    sysConfig.load();
    sysConfig.WiFiEnabled = true;
    sysConfig.SendUpdateRate = 5000;

    ioConfig.ADC4Config = ADC_CONFIG_ADC;
    ioConfig.ADC4Name = "in4";
    ioConfig.ADC4Scaler = 1;
    ioConfig.ADC4Calibration = 1;
    ioConfig.ADC4Zero = 0;

    ioConfig.ADC5Config = ADC_CONFIG_ADC;
    ioConfig.ADC5Name = "out5";
    ioConfig.ADC5Scaler = 1;
    ioConfig.ADC5Calibration = 1;
    ioConfig.ADC5Zero = 0;

    ioConfig.GPIO1Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO1Name = "lowpressure";
    configPins.InvertGpio1 = false;

    ioConfig.GPIO2Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO2Name = "flow";
    configPins.InvertGpio1 = true;

    ioConfig.GPIO4Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO4Name = "highpressure";
    configPins.InvertGpio4 = false;

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

    onOffDetector.setup(&ioConfig);
    relayManager.setup(&ioConfig);

    state.registerBool("heateron", false);
    _heaterOn = state.getBool("heateron");

    adc.setUseFiltering(24, 2);
    adc.setBankEnabled(1, true);
    adc.setBankEnabled(2, true);
    adc.setup(&ioConfig);

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

void loop()
{
    console.loop();
    ledManager.loop();
    hal.loop();
    BT.update();
    wifiMgr.loop();

    if (state.OTAState == 100)
    {
        ota.downloadOverWiFi();
    }

    if (wifiMgr.isConnected())
    {
        if (!webServerSetup)
        {
            setupWebServer();
        }

        webServer->handleClient();
    }

    delay(1000);
    wifiMQTT.loop();

    adc.loop();
    onOffDetector.loop();

    float temperature = atof(ioValues.getValue(3).c_str()) * 32.0f;

    if(_heaterOn) {
        if(temperature > 91) {
            _shouldHeat = false;
        }
        else if(temperature < 90) {
            _shouldHeat = true;
        }
    }
    else {
        _shouldHeat = false;
    }

    String err = "none";

    if (ioValues.getValue(1 + 8) == "1")
    {
        relayManager.setRelay(0, _shouldHeat);
        relayManager.setRelay(1, _shouldHeat);
        relayManager.setRelay(2, _shouldHeat);
    }
    else
    {
        relayManager.setRelay(0, 0);
        relayManager.setRelay(1, 0);
        relayManager.setRelay(2, 0);
        err = "lowpressure";
    }

    if (wifiMgr.isConnected())
    {
        if (nextSend < millis() || nextSend == 0)
        {
            

            String msg = "{'heaterOn':" + String(_heaterOn) + 
                         ",'heating':" + String(_shouldHeat) + 
                         ",'in':" + String(temperature) + 
                         ",'out':" +ioValues.getValue(4) + 
                         ",'lowPressure':" + (ioValues.getValue(0 + 8) == "1" ? "'ok'" : "'warning'") + 
                         ",'flow':" +(ioValues.getValue(1 + 8) == "1" ? "'ok'" : "'warning'") + 
                         ",'highpressure':" +(ioValues.getValue(3 + 8) == "1" ? "'ok'" : "'warning'") + 
                         ",'ipaddress':'" +wifiMgr.getIPAddress() + "','err':'" + err + "'}";

            wifiMQTT.publish("poolctrlr/poolheater/" + sysConfig.DeviceId, msg);
            console.println(msg);

            wifiMQTT.sendIOValues(&ioValues);
            wifiMQTT.sendRelayStatus(&relayManager);
            nextSend = millis() + sysConfig.SendUpdateRate;
            console.setVerboseLogging(true);
            adc.debugPrint();
            onOffDetector.debugPrint();
            relayManager.debugPrint();
        }
    }
}