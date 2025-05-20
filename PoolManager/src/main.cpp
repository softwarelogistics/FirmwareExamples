#define RELAY_BRD_V1
#define BT_ENABLED

#include <Arduino.h>
#include <StringSplitter.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include <NuvIoT.h>
#include <MedianFilter.h>
#include <WebSocketsServer.h>

#include <uri/UriRegex.h>

#define FIRMWARE_VERSION "3.0.0"
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
WebSocketsServer webSocket = WebSocketsServer(81);


#define homePage  "<!doctype html>\
<html lang=\"en\" data-beasties-container>\
<head>\
  <meta charset=\"utf-8\">\
  <title>PoolMgr</title>\
  <base href=\"/\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
  <link rel=\"icon\" type=\"image/png\" href=\"https://www.nuviot.com/images/nuviotico.png\">\
  <script src=\"https://kit.fontawesome.com/64cf30dd1a.js\" crossorigin=\"anonymous\"></script>\
  <script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>\
  <script src=\"https://cdn.jsdelivr.net/npm/popper.js@1.14.3/dist/umd/popper.min.js\" integrity=\"sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49\" crossorigin=\"anonymous\"></script>\
  <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">\
  <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/js/bootstrap.min.js\" integrity=\"sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy\" crossorigin=\"anonymous\"></script>\
  <link rel=\"stylesheet\" href=\"https://nuviot.blob.core.windows.net/cdn/sites/pool-mgr/styles.css\"></head>\
  <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\">\
  <link href=\"https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500&display=swap\" rel=\"stylesheet\">\
  <link href=\"https://fonts.googleapis.com/icon?family=Material+Icons\" rel=\"stylesheet\">\
  <script src=\"https://maps.googleapis.com/maps/api/js?key=AIzaSyCAfoq8E_G7iXNmOlNxxa74OPVKyqBAq18\"></script>\
<body>\
  <app-root></app-root>\
  <script src=\"https://nuviot.blob.core.windows.net/cdn/sites/pool-mgr/polyfills.js\" type=\"module\"></script>\
  <script src=\"https://nuviot.blob.core.windows.net/cdn/sites/pool-mgr/main.js\" type=\"module\"></script>\
</body>\
</html>"


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:

            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
    
		        webSocket.sendTXT(num, "{\"connect\":true}");
                console.print("client web connected"); 
            }
            break;
        case WStype_TEXT:
        
            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
        
            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
	case WStype_ERROR:			
	case WStype_FRAGMENT_TEXT_START:
	case WStype_FRAGMENT_BIN_START:
	case WStype_FRAGMENT:
	case WStype_FRAGMENT_FIN:
	    break;
    }

}

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


#define HEATER_MODE_OFF 0
#define HEATER_MODE_POOL 1
#define HEATER_MODE_HOT_TUB 2

void setup()
{
    configureFileSystem();

    state.init(FW_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);

    configureConsole();

    console.setVerboseLogging(false);
    welcome(FW_SKU, FIRMWARE_VERSION);

    initPins();

    writeConfigPins();

    ioConfig.load();
    sysConfig.load();
    sysConfig.Port = 1883;
    sysConfig.SendUpdateRateMS = 5000;

    // ioConfig.GPIO1Config = GPIO_CONFIG_DBS18;
    // ioConfig.GPIO1Name = "temperatureIn";

    // ioConfig.GPIO2Config = GPIO_CONFIG_DBS18;
    // ioConfig.GPIO2Name = "temperatureOut";

    
    ioConfig.GPIO1Config = GPIO_CONFIG_NONE;
    ioConfig.GPIO1Name = "temperatureIn";

    ioConfig.GPIO2Config = GPIO_CONFIG_NONE;
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


    String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Pool Manager" : sysConfig.DeviceId);
#ifdef BT_ENABLED       
    BT.begin(btName.c_str(), FW_SKU);
#endif

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

    sysConfig.WiFiSSID = "SLManCave";
    wifiMgr.setup();
}

int idx = 0;

long nextSend = 0;

boolean webServerSetup = false;

String getJSONState() {
    String msg = "{\"heating\":" + String(_shouldHeat) +
                    ",\"poolMode\":" + (poolMode == 0 ? "\"off\"" : poolMode == 1 ? "\"pool\"" : "\"spa\"") +
                    ",\"spaSetPoint\":" + String(spaSetPoint) +
                    ",\"poolSetPoint\":" + String(poolSetPoint) +
                    ",\"in\":" + String(temperatureIn) +
                    ",\"out\":" + String(temperatureOut) +
                    ",\"lowPressure\":" + (lowPressure ? "\"ok\"" : "\"warning\"") +
                    ",\"flow\":" + (hasFlow ? "\"ok\"" : "\"warning\"") +
                    ",\"highPressure\":" + (highPressure ? "\"ok\"" : "\"warning\"") +
                    ",\"ipaddress\":\"" + wifiMgr.getIPAddress() + "\"" +
                    ",\"err\":\"" + _lastError + "\"}";
    return msg;
}

void setupWebServer(){

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    webServer->enableCORS(true);
    webServer->on("/api/state", HTTP_GET, []() {
     //   webServer->sendHeader("Access-Control-Allow-Origin", "*");
        webServer->send(200, "application/json", getJSONState());
    });

    webServer->on("/api/mode/off", HTTP_GET, []() {
        poolMode = HEATER_MODE_OFF;
        state.updateProperty("Integer", "poolmode", "0");
        webServer->send(200, "application/json", getJSONState());
    });
 
    webServer->on("/api/mode/pool", HTTP_GET, []() {
        poolMode = HEATER_MODE_POOL;
        state.updateProperty("Integer", "poolmode", "1");
        webServer->send(200, "application/json", getJSONState());
    });

    webServer->on("/api/mode/spa", HTTP_GET, []() {
        poolMode = HEATER_MODE_HOT_TUB;
        state.updateProperty("Integer", "poolmode", "2");
        webServer->send(200, "application/json", getJSONState());
    });

    webServer->onNotFound([]() {
        String uri = webServer->uri();
        if (uri.startsWith("/api/setpoint/pool")) {
            String valueStr = uri.substring(strlen("/api/setpoint/pool/"));
            float setpoint = valueStr.toFloat();    
            if(setpoint > 60 && setpoint < 110) {
                poolSetPoint = setpoint;
                state.updateProperty("Decimal", "poolsetpoint", String(poolSetPoint));
                webServer->send(200, "application/json", getJSONState());
            }
            else {
                webServer->send(400, "application/json", "{\"error\":\"Invalid setpoint\"}");
            }
        } else if (uri.startsWith("/api/setpoint/spa")) {
           String valueStr = uri.substring(strlen("/api/setpoint/spa/"));
            float setpoint = valueStr.toFloat();    
            if(setpoint > 60 && setpoint < 110) {
                spaSetPoint = setpoint;
                state.updateProperty("Decimal", "hottubsetpoint", String(spaSetPoint));
                webServer->send(200, "application/json", getJSONState());
            }
            else {
                webServer->send(400, "application/json", "{\"error\":\"Invalid setpoint\"}");
            }
        }
        else
        {
            webServer->send(200, "text/html", homePage);
        }
    });

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
        _lastError = "noflow";
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
            if(MDNS.begin(sysConfig.DeviceId.c_str()))
            {
                console.println("MDNS responder started on " + sysConfig.DeviceId);
            }
            else
            {
                console.println("Error setting up MDNS responder!");
            }
            setupWebServer();
        }

        webServer->handleClient();
        webSocket.loop();
    }

    if (nextSend < millis() || nextSend == 0)
    {
        nextSend = millis() + sysConfig.SendUpdateRateMS;
        String json = getJSONState();        
        webSocket.broadcastTXT(json);
        sendUpdate();
    }
}