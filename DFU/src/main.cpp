#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "CIM001"
#define FIRMWARE_VERSION "0.3.0"

bool running = true;

void cmdCallback(String cmd)
{
  if (cmd == "exit")
  {
    running = false;
  }
  else if (cmd == "restart")
  {
    running = true;
  }
}

void setup()
{
  initPins();

  configureConsole();
  
  console.registerCallback(cmdCallback);

  configureModem();

  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "dfu001", 010);

  sysConfig.WiFiEnabled = false;
  sysConfig.CellEnabled = true;
  sysConfig.Commissioned = true;
  sysConfig.SrvrHostName = "pt1.seawolf.iothost.net";
  sysConfig.SrvrUID = "seawolf";
  sysConfig.SrvrPWD = "4NuvIoT!";
  sysConfig.DeviceId = "floyd";
  sysConfig.PingRate = 5;
  sysConfig.SendUpdateRate = 2500;

  console.setVerboseLogging(false);
  connect();
}

int nextPrint = 0;
int idx = 0;

void loop(){
  commonLoop();  

  console.setVerboseLogging(false);
  
  if (nextPrint < millis() && running){
    nextPrint = millis() + sysConfig.SendUpdateRate;    
    //mqttPublish("test/" + sysConfig.DeviceId, "{'hi':'bob'}");
    mqttPublish("/test/" + sysConfig.DeviceId);
  }
}