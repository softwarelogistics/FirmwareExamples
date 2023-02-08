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

#define FIRMWARE_VERSION "0.2.0"
#define HARDWARE_REVISION "5"
#define FW_SKU "POOL001"

#include <PageHandler.h>
#include "pages.h"

/* 
 * Temperature 2.62 - 79.7 (in)
 * Temperature 2.53 =  84.0
 * Temperature 2.59 =  85.5
 * Temperature 2.76 =  92.2
 * Temperature 2.86 =  94
 *             2.97 =  101.5
 * 
 */

boolean _shouldHeat = false;

class Thermistor
{
};

WebServer *webServer = new WebServer(80);

void handleTopic(String topic)
{
}

void redirectToSensorPage()
{
    webServer->sendHeader("Location", String("/sensor"), true);
    webServer->send(302, "text/plain");
}

void handleApi(String noun, String verb)
{
    redirectToSensorPage();
    _shouldHeat = verb == "on";
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
                    _shouldHeat = splitter->getItemAtIndex(2) == "on";
                }
            }
        }

        delete splitter;

        sendSensorData(&ioValues, FIRMWARE_VERSION);
        return true;
    }
} pageHandler;

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
    ioConfig.GPIO2Name = "highpressure";
    configPins.InvertGpio1 = true;    
    
    ioConfig.GPIO4Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO4Name = "flow";
    configPins.InvertGpio4 = true;

    ioConfig.Relay1Name = "heat1";
    ioConfig.Relay1Enabled = true;
    ioConfig.Relay2Name = "heat2";
    ioConfig.Relay2Enabled = true;
    ioConfig.Relay3Name = "fan";
    ioConfig.Relay3Enabled = true;


    String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Pool Manager" : sysConfig.DeviceId);
    BT.begin(btName.c_str(), FW_SKU);

    ledManager.setup(&ioConfig);
    ledManager.setOnlineFlashRate(1);
    ledManager.setErrFlashRate(0);

    configureI2C();

    onOffDetector.setup(&ioConfig);
    relayManager.setup(&ioConfig);

    adc.setUseFiltering(12, 2);
    adc.setBankEnabled(1, true);
    adc.setBankEnabled(2, true);
    adc.setup(&ioConfig);

    wifiMgr.setup();
}

int idx = 0;

long nextSend = 0;

void loop()
{
    console.loop();
    ledManager.loop();
    wifiMQTT.loop();
    
    adc.loop();
    onOffDetector.loop();    

    if (wifiMgr.isConnected())
    {
        if (!webServerSetup)
        {
            setupWebServer();
        }

        webServer->handleClient();
    }

    relayManager.setRelay(0, _shouldHeat);
    relayManager.setRelay(1, _shouldHeat);
    relayManager.setRelay(2, _shouldHeat);    

    hal.loop();
    BT.update();

    if(nextSend < millis() || nextSend == 0)
    {
        String msg = "{'heating':" + String(_shouldHeat) + "," 
                      "'in':" + ioValues.getValue(0 + 8) + ","
                      "'out':" + ioValues.getValue(1 + 8) + ","
                      "'flow':" + ioValues.getValue(3 + 9) + ","
                      "'lowPressure':" + ioValues.getValue(3) + ","                      
                      "'highPressure':" + ioValues.getValue(4) + ","
                      "'ipaddress':'" + wifiMgr.getIPAddress() + "'}";


        wifiMQTT.publish("poolctrlr/" + sysConfig.DeviceId + "/status", msg);
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