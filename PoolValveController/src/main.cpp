#define ESP32_Relay_X8

#include <Arduino.h>

#include <NuvIoT.h>
//#include <WebServer.h>
#include <StringSplitter.h>

#include "EEPROM.h"

#include "valve.h"
#include "pages.h"

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

const char *host = "pool-valves";
const char *ssid = "CasaDeWolf";
const char *password = "TheWolfBytes";

// GPIO32, GPIO33, GPIO25, GPIO26, GPIO27, GPIO14, GPIO12 and GPIO13.

Valve jets(&console, &relayManager, &state, &onOffDetector, "jets", 0, 1, 0, "spa", "both", "normal");
Valve source(&console, &relayManager, &state, &onOffDetector, "source", 2, 3, 1, "pool", "both", "spa");
Valve output(&console, &relayManager, &state, &onOffDetector, "output", 4, 5, 2, "pool", "both", "spa");

bool webServerSetup = false;

const int MOTOR_TEMP = A0;

unsigned long next_send = 0;



void handleTopic(String device, String action)
{
  console.println("Handling topic  " + String(device) + ", " + String(action));

  if (device == "source" && action == "pool")
    source.setCurrentPosition(currentPosition_0);
  if (device == "source" && action == "both")
    source.setCurrentPosition(currentPosition_90);
  if (device == "source" && action == "spa")
    source.setCurrentPosition(currentPosition_180);
  if (device == "source" && action == "reset")
    source.reset();

  if (device == "output" && action == "pool")
    output.setCurrentPosition(currentPosition_0);
  if (device == "output" && action == "spa")
    output.setCurrentPosition(currentPosition_180);
  if (device == "output" && action == "both")
    output.setCurrentPosition(currentPosition_90);
  if (device == "output" && action == "reset")
    output.reset();

  if (device == "spa" && action == "normal")
    jets.setCurrentPosition(currentPosition_180);
  if (device == "spa" && action == "both")
    jets.setCurrentPosition(currentPosition_90);
  if (device == "spa" && action == "jets")
    jets.setCurrentPosition(currentPosition_0);
  if (device == "spa" && action == "reset")    
    jets.reset();
}

void commandHandler(String topic, byte *buffer, size_t len){
  if(topic == "pool") {
    source.setCurrentPosition(currentPosition_0);
    output.setCurrentPosition(currentPosition_0);
  }
  else if(topic == "spa") {
    source.setCurrentPosition(currentPosition_180);
    output.setCurrentPosition(currentPosition_180);
    jets.setCurrentPosition(currentPosition_180);
  }
  else if(topic == "poolandspa") {
    source.setCurrentPosition(currentPosition_180);
    output.setCurrentPosition(currentPosition_90);
    jets.setCurrentPosition(currentPosition_180);
  }
  else if(topic == "jets") {
    jets.setCurrentPosition(currentPosition_0);
  }
  else if(topic == "calibrate") {
    source.calibrate();
    output.calibrate();
    jets.calibrate();
  }

  console.println("Command Received: " + topic); 
}

void handleSetTiming(String port, String timing)
{
  EEPROM.begin(12);
  uint16_t newTiming = atoi(timing.c_str()) * 1000;

  if (port == "source")
  {
    source.setTiming(newTiming);
    EEPROM.put(sizeof(uint16_t), newTiming);
  }
  else if (port == "output")
  {
    output.setTiming(newTiming);
    EEPROM.put(sizeof(uint16_t) * 2, newTiming);
  }
  else if (port == "jets")
  {
    jets.setTiming(newTiming);
    EEPROM.put(sizeof(uint16_t) * 4, newTiming);
  }
  EEPROM.end();
}

/*
class PoolPageHandler : public RequestHandler
{
  bool canHandle(HTTPMethod method, String uri)
  {
    console.println("webserver=handle; //  Method " + String(method) + " - " + uri);
    return true;
  }

  boolean handleSet(StringSplitter *items)
  {
    if (items->getItemCount() > 2)
    {
      String segment = items->getItemAtIndex(1);
      console.println("webserver=handlesegment; // parent = set; " + segment);

      if (segment == "relay")
      {
        if (items->getItemCount() > 4)
        {
          int relayIdx = atoi(items->getItemAtIndex(2).c_str());
          bool action = items->getItemAtIndex(3) == "on";
          relayManager.setRelay(relayIdx - 1, action);
          sendTestPage(FIRMWARE_VERSION);
        }
        else
        {
          sendHomePage(FIRMWARE_VERSION);
        }
      }
      else if(segment == "timing")
      {
        handleSetTiming(items->getItemAtIndex(2), items->getItemAtIndex(3));
        redirectToValvesPage();
      }
      else
      {
        handleTopic(items->getItemAtIndex(1), items->getItemAtIndex(2));
        redirectToValvesPage();
      }
    }
    else
    {
      sendHomePage(FIRMWARE_VERSION);
    }

    return true;
  }

  bool handle(WebServer &server, HTTPMethod requestMethod, String requestUri)
  {
    StringSplitter *splitter = new StringSplitter(requestUri, '/', 6);
    uint8_t itemCount = splitter->getItemCount();

    if (itemCount > 0)
    {
      String segmentOne = splitter->getItemAtIndex(0);

      if (segmentOne == "set")
        handleSet(splitter);
      else if (segmentOne == "relays")
      {
        sendTestPage(FIRMWARE_VERSION);
      }
      else if (segmentOne == "valves")
      {
        sendValveControlPage(FIRMWARE_VERSION);
      }   
      else {
        sendHomePage(FIRMWARE_VERSION);
      }
    }
    else
    {
      sendHomePage(FIRMWARE_VERSION);
    }

    delete splitter;

    return true;
  }
} pageHandler;



void handleRoot()
{
  httpServer->send(200, "text/plain", "hello from esp8266!");
}

void setupWebServer()
{
  httpServer->addHandler(&pageHandler);
  httpServer->begin();
  webServerSetup = true;
}*/

void initTimings() {
  EEPROM.begin(12);
  uint16_t initValue;

  EEPROM.get(0, initValue);
  if(initValue != 0x5432){
    int defaultValue = 16000;
    console.println("EEPROM Not set, initialize values");
    EEPROM.put(0, (uint16_t)0x5432);
    EEPROM.put(sizeof(uint16_t), defaultValue);
    EEPROM.put(sizeof(uint16_t) * 2, defaultValue);
    EEPROM.put(sizeof(uint16_t) * 4, defaultValue);
  }
  else {
    console.println("EEPROM has valid content");
  }

  uint16_t timing = 0;

  EEPROM.get(sizeof(uint16_t), timing);  
  source.setTiming(timing);

  EEPROM.get(sizeof(uint16_t) * 2, timing);
  output.setTiming(timing);

  EEPROM.get(sizeof(uint16_t) * 4, timing);
  jets.setTiming(timing);

  EEPROM.end();
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
  sysConfig.WiFiEnabled = true;
  sysConfig.SendUpdateRateMS = 5000;

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

  initTimings();  
  jets.init();
  source.init();
  output.init();

  wifiMgr.setup();

  setCommandHandler(commandHandler);
}

void loop(void)
{
  int motorTempCount = analogRead(A0);

  jets.Update();
  source.Update();
  output.Update();

  relayManager.loop();
  onOffDetector.loop();

  commonLoop();

  // if (wifiMgr.isConnected())
  // {
  //   if (!webServerSetup)
  //   {
  //     setupWebServer();
  //   }

  //   httpServer->handleClient();
  // }

  if (next_send < millis())
  {
    String topic = "pool/valvestat/" + String(sysConfig.DeviceId);
    String msg = "{\"deviceId\":\"" +
                 String(sysConfig.DeviceId) +
                 "\",\"ipaddress\":\"" +
                 WiFi.localIP().toString() +
                 "\",\"source\":\"" +
                 source.getStatus() +
                 "\",\"output\":\"" +
                 output.getStatus() + 
                 "\",\"spa\":\"" +
                 jets.getStatus() + 
                 "\", \"motorTemp\":" + 
                 String(motorTempCount) + "}";

    mqttPublish(topic, msg);

    console.println(msg);
    next_send = millis() + sysConfig.SendUpdateRateMS;
  }
}
