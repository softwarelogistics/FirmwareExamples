#define ARDUHAL_LOG_LEVEL ARDUHAL_LOG_LEVEL_NONE

#define ESP32_Relay_X8
#define BT_ENABLED
#include <Arduino.h>

#include <ESPmDNS.h>
#include <NuvIoT.h>

#include "EEPROM.h"

#include "valve.h"
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ValveConfig.h>

#define DEFAULT_DEVICE_TYPE_ID "272EC811A9074F8C8CA72D2EAB7D3B44"

WebServer *webServer = new WebServer(80);
WebSocketsServer *webSocket = new WebSocketsServer(81);


// Programming Interface
// Back Header
// 1 - DTR - White
// 2 - GND - Brown
// 3 - N/C
// 4 - From USB - Orange
// 5 - To USB - Yellow
// 6 - Red - +V5

// U3
// 2 - RTS Green

//#include <uri/UriRegex.h>

#define FW_SKU "PLVL001"
#define FIRMWARE_VERSION "0.8.2"
#define HARDWARE_REVISION "COT-01"

#define CURRENT_STATE_INDEX 0x5432

const char *host = "pool-valves";
const char *ssid = "CasaDeWolf";
const char *password = "TheWolfBytes";

String homePage  = "<!doctype html>\
<html lang=\"en\" >\
<head>\
  <meta charset=\"utf-8\">\
  <title>Pool Valve Controller</title>\
  <base href=\"/\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
  <link rel=\"icon\" type=\"image/png\" href=\"https://www.nuviot.com/images/nuviotico.png\">\
  <script src=\"https://kit.fontawesome.com/64cf30dd1a.js\" crossorigin=\"anonymous\"></script>\
  <script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>\
  <script src=\"https://cdn.jsdelivr.net/npm/popper.js@1.14.3/dist/umd/popper.min.js\" integrity=\"sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49\" crossorigin=\"anonymous\"></script>\
  <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">\
  <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/js/bootstrap.min.js\" integrity=\"sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy\" crossorigin=\"anonymous\"></script>\
  <link rel=\"stylesheet\" href=\"https://nuviot.blob.core.windows.net/devicetypeapps/[DEVICETYPEID]/styles.css\"></head>\
  <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\">\
  <link href=\"https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500&display=swap\" rel=\"stylesheet\">\
  <link href=\"https://fonts.googleapis.com/icon?family=Material+Icons\" rel=\"stylesheet\">\
  <script src=\"https://maps.googleapis.com/maps/api/js?key=AIzaSyCAfoq8E_G7iXNmOlNxxa74OPVKyqBAq18\"></script>\
<body>\
  <app-root></app-root>\
  <script src=\"https://nuviot.blob.core.windows.net/devicetypeapps/[DEVICETYPEID]/polyfills.js\" type=\"module\"></script>\
  <script src=\"https://nuviot.blob.core.windows.net/devicetypeapps/[DEVICETYPEID]/main.js\" type=\"module\"></script>\
</body>\
</html>";

// GPIO32, GPIO33, GPIO25, GPIO26, GPIO27, GPIO14, GPIO12 and GPIO13.

#define VALVE_COUNT 3

ValvesConfig valveConfig(&console, VALVE_COUNT);


Valve *valves[VALVE_COUNT] = {new Valve(), new Valve(), new Valve()};

bool webServerSetup = false;

const int MOTOR_TEMP = A0;

unsigned long next_send = 0;

void handleTopic(String device, String action){
  console.println("Handling topic  " + String(device) + ", " + String(action));

  for(uint8_t i = 0; i < VALVE_COUNT; i++)
  {
    console.println("Found valve " + String(valves[i]->getKey()) + " at index " + String(i));
 
    if(valves[i]->getKey() == device){
      valves[i]->setCurrentPosition(action);
      return;
    }
  }

   console.println("Could not find device  " + String(device));


  // if (device == "source" && action == "pool")
  //   source.setCurrentPosition(currentPosition_0);
  // if (device == "source" && action == "both")
  //   source.setCurrentPosition(currentPosition_90);
  // if (device == "source" && action == "spa")
  //   source.setCurrentPosition(currentPosition_180);
  // if (device == "source" && action == "reset")
  //   source.reset();

  // if (device == "output" && action == "pool")
  //   output.setCurrentPosition(currentPosition_0);
  // if (device == "output" && action == "spa")
  //   output.setCurrentPosition(currentPosition_180);
  // if (device == "output" && action == "both")
  //   output.setCurrentPosition(currentPosition_90);
  // if (device == "output" && action == "reset")
  //   output.reset();

  // if (device == "spa" && action == "normal")
  //   jets.setCurrentPosition(currentPosition_180);
  // if (device == "spa" && action == "both")
  //   jets.setCurrentPosition(currentPosition_90);
  // if (device == "spa" && action == "jets")
  //   jets.setCurrentPosition(currentPosition_0);
  // if (device == "spa" && action == "reset")    
  //   jets.reset();
}

void setMode(String mode) { 
  // if(mode == "pool") {
  //   source.setCurrentPosition(currentPosition_0);
  //   output.setCurrentPosition(currentPosition_0);
  // }
  // else if(mode == "spa") {
  //   source.setCurrentPosition(currentPosition_180);
  //   output.setCurrentPosition(currentPosition_180);
  //   jets.setCurrentPosition(currentPosition_180);
  // }
  // else if(mode == "poolandspa") {
  //   source.setCurrentPosition(currentPosition_180);
  //   output.setCurrentPosition(currentPosition_90);
  //   jets.setCurrentPosition(currentPosition_180);
  // }
  // else if(mode == "jets") {
  //   jets.setCurrentPosition(currentPosition_0);
  // }
  // else if(mode == "calibrate") {
  //   source.calibrate();
  //   output.calibrate();
  //   jets.calibrate();
  // }
}

void commandHandler(String topic, byte *buffer, size_t len){
  setMode(topic);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:

            break;
        case WStype_CONNECTED:{
    	        webSocket->sendTXT(num, "{\"messageId\":\"connect\",\"deviceId\":\"" + sysConfig.DeviceId + "\",\"id\":\"" + sysConfig.Id + "\",\"deviceType\":\"" + sysConfig.DeviceTypeId + "\",\"orgId\":\"" + sysConfig.OrgId +  "\",\"repoId\":\"" + sysConfig.RepoId + "\",\"customerId\":\"" + sysConfig.CustomerId + "\"}");
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


String getJSONState(){
    String json = "{\"messageId\":\"state\"";  
    for(uint8_t i = 0; i < VALVE_COUNT; i++)
    {
        json += ",\"" + valveConfig.getValves()[i].Key + "\":\"" + valves[i]->getStatusKey() + "\"";
    }

    json += "}";
    return json;
} 

void setupWebServer(){

    webSocket->begin();
    webSocket->onEvent(webSocketEvent);

    webServer->enableCORS(true);
    webServer->on("/api/state", HTTP_GET, []() {
        webServer->send(200, "application/json", getJSONState());
    });

    webServer->on("/api/valve/config", HTTP_GET, []() {
        webServer->send(200, "application/json", valveConfig.toJSON());
    });

    webServer->onNotFound([]() {
        String uri = webServer->uri();
        if (uri.startsWith("/api/valve")) {
            String valueStr = uri.substring(strlen("/api/valve/"));
            String valve = valueStr.substring(0, valueStr.indexOf('/'));
            String action = valueStr.substring(valueStr.indexOf('/') + 1);
            console.println("valve: " + valve + ", " + action);
            handleTopic(valve, action);
            webServer->send(200, "application/json", getJSONState());
        }
        if (uri.startsWith("/api/mode")) {
            String mode = uri.substring(strlen("/api/mode/"));
            setMode(mode);
            webServer->send(200, "application/json", getJSONState());
        }
        else if(uri.startsWith("/api/config/valve")) {
            String valueStr = uri.substring(strlen("/api/config/valve/"));
            String port = valueStr.substring(0, valueStr.indexOf('/'));
            String field = valueStr.substring(valueStr.indexOf('/') + 1, valueStr.lastIndexOf('/'));
            String value = valueStr.substring(valueStr.lastIndexOf('/') + 1);
            console.println("valuestr: " + valueStr + ";port: " + port + "; field: " + field + "; value: " + value);
            
            ValveConfig *config = valveConfig.getValve(port);
            if(config == NULL){
                console.println("Valve not found: " + port);
                webServer->send(404, "application/json", "{\"error\":\"Valve not found\"}");
                return;
            } 
            else {
              config->writeField(field, value);
              valveConfig.write();
              webServer->send(200, "application/json", valveConfig.toJSON());
     
            }

           // handleSetTiming(port, timing);
        }
        else{
            webServer->send(200, "text/html", homePage);
        }
    });

    webServer->begin();

    webServerSetup = true;
}


void setup(void)
{
  delay(1000);
  initPins();

  configureConsole();

  configureFileSystem();

  state.init(FW_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "plvl001", 010);

  ioConfig.load();
  sysConfig.load();
  valveConfig.load();


 

  if(sysConfig.DeviceTypeId == NULL || sysConfig.DeviceTypeId == ""){
      sysConfig.DeviceTypeId = DEFAULT_DEVICE_TYPE_ID;
      sysConfig.write();
  }
  
  if(sysConfig.SendUpdateRateMS > 5000){
    sysConfig.SendUpdateRateMS = 5000;
    sysConfig.write();
  }

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Valve Ctrlr" : sysConfig.DeviceId);
  BT.begin(btName.c_str(), FW_SKU);

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);
  ledManager.setErrFlashRate(0);

  onOffDetector.setup(&ioConfig);

  ioConfig.Relay1Name = "jet_pwr";
  ioConfig.Relay1Enabled = true;
  ioConfig.Relay2Name = "jet_dir";
  ioConfig.Relay3Enabled = true;
  ioConfig.Relay3Name = "src_pwr";
  ioConfig.Relay3Enabled = true;
  ioConfig.Relay4Name = "src_dir";
  ioConfig.Relay4Enabled = true;
  ioConfig.Relay5Name = "out_pwr";
  ioConfig.Relay5Enabled = true;
  ioConfig.Relay6Name = "our_dir";
  ioConfig.Relay6Enabled = true;
  ioConfig.Relay7Name = "pool_light";
  ioConfig.Relay7Enabled = true;
  ioConfig.Relay8Name = "spa_light";
  ioConfig.Relay8Enabled = true;

  relayManager.setup(&ioConfig);

  ioConfig.GPIO1Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO1Name = "jet_center";  
  configPins.InvertGpio1 = true;
  
  ioConfig.GPIO2Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO2Name = "src_center";
  configPins.InvertGpio2 = true;

  ioConfig.GPIO3Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO3Name = "out_center";
  configPins.InvertGpio3 = true;  
  onOffDetector.setup(&ioConfig);

  wifiMgr.setup();

 for(uint8_t i = 0; i < VALVE_COUNT; i++){
    valves[i]->init(&console, &relayManager, webSocket, &valveConfig, i);
  }

  setCommandHandler(commandHandler);
}

void loop(void){
  for(uint8_t i = 0; i < VALVE_COUNT; i++)
  {
    valves[i]->Update();
  } 
  
  relayManager.loop();
  onOffDetector.loop();

  commonLoop();
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
      webSocket->loop();
  }

  if (next_send < millis()){
    next_send = millis() + sysConfig.SendUpdateRateMS;
    String json = getJSONState();
    if(wifiMQTT.isConnected()){
      String topic = "pool/valvestat/" + String(sysConfig.DeviceId);
      mqttPublish(topic, getJSONState());
    }
    
    webSocket->broadcastTXT(json);
    console.println(json);
  }
}
