#include <Arduino.h>
#include <NuvIoT.h>
#include "../../common/set_secrets.h"

#define EXAMPLE_SKU "BLE-DFU"
#define FIRMWARE_VERSION "0.3.0"

void setup()
{
  initPins();
  configureFileSystem();
  configureConsole();
 
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "dfu001", 010);

  sysConfig.WiFiEnabled = true;
  setSysConfigSecrets(&sysConfig);
  
  sysConfig.CellEnabled = false;
  sysConfig.Commissioned = true;
  sysConfig.DeviceId = "dfu001";
  sysConfig.PingRate = 60;
  sysConfig.SendUpdateRate = 2500;

  console.setVerboseLogging(false);
  BT.begin("NuvIoT - BLE-DFU Sample", "123456790");
}

int nextPrint = 0;
int idx = 0;

void loop(){
  commonLoop();     
}