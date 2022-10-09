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


#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_REVISION "5"
#define FW_SKU "POOL001"

#include <PageHandler.h>
#include "pages.h"

boolean _shouldHeat = false;


WebServer *webServer = new WebServer(80);

void handleTopic(String topic) {

}

void redirectToSensorPage() {
  webServer->sendHeader("Location", String("/sensor"), true);
  webServer->send(302, "text/plain");
}

void handleApi(String noun, String verb) {
    redirectToSensorPage();
    _shouldHeat = verb == "on";
}

class PoolPageHandler : public RequestHandler {
    bool canHandle(HTTPMethod method, String uri) {
    return true;
  }

  bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {    
    StringSplitter *splitter = new StringSplitter(requestUri, '/', 6);
    uint8_t itemCount = splitter->getItemCount();

    for(int i = 0; i < itemCount; i++){
        String item = splitter->getItemAtIndex(i);
        console.println(String(i + 1) + ". " + item);
    }

    if(itemCount > 2){
        if(splitter->getItemAtIndex(0) == "api"){
            if(splitter->getItemAtIndex(1) == "heater") {
                _shouldHeat = splitter->getItemAtIndex(2) == "on";
            }
        }
    }

    delete splitter;

    sendSensorData();
    return true;
  }
} pageHandler;

boolean webServerSetup = false;

void setupWebServer(){
  //webServer->on("/", HTTP_GET, []() { webServer->sendHeader("Connection", "close"); webServer->send(200, "text/html", loginIndex); });
  //webServer->on("/serverIndex", HTTP_GET, [](){ webServer->sendHeader("Connection", "close"); webServer->send(200, "text/html", serverIndex); });
  //webServer->on("/status", HTTP_GET, []() { sendSensorData(); });
  //webServer->on("/sensor", sendSensorData);
  //webServer->on("/heater/on", HTTP_GET, [](){  handleApi("heater", "on"); });
  //webServer->on("/heater/off", HTTP_GET, [](){ handleApi("heater", "off");  });

  webServer->addHandler(&pageHandler);
  //webServer->on(UriRegex("\/set\/([a-z]+)\/([a-z]+)"), HTTP_GET, [&]() { handleTopic(webServer->pathArg(0) + "/" + webServer->pathArg(1)); redirectToSensorPage();} );
  webServer->begin();

  webServerSetup = true;
}


void setup(){
    delay(1000);

    configureFileSystem();
    
    state.init(FW_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);

    configureConsole();

    ioConfig.load();
    sysConfig.load();
    sysConfig.WiFiEnabled = true;
    sysConfig.SendUpdateRate = 1000;

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

    ioConfig.GPIO2Config = GPIO_CONFIG_INPUT;
    ioConfig.GPIO2Name = "highpressure";

    ioConfig.Relay1Name = "heat1";
    ioConfig.Relay2Name = "heat2";
    
    initPins();

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

void loop(){
    console.loop();
    ledManager.loop();
    wifiMQTT.loop();
    adc.loop();
    onOffDetector.loop();
    console.setVerboseLogging(true);

    adc.debugPrint();
    onOffDetector.debugPrint();


    if(wifiMgr.isConnected()) {
        if(!webServerSetup ){
            setupWebServer();
        }

        webServer->handleClient();
    }

    if(_shouldHeat) {
        relayManager.setRelay(0, true);
        relayManager.setRelay(1, true);
    }
    else {
        relayManager.setRelay(0, false);
        relayManager.setRelay(1, false);
    }

    relayManager.debugPrint();

    hal.loop();
    BT.update();
    delay(1000);
}