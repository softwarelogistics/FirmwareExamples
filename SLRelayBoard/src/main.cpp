#define RELAY_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#include <Wire.h>

#define TEMP_SNSR_SKU "Relay Board"
#define FIRMWARE_VERSION "0.5.1"
#define HARDWARE_REVISION "5.0"

byte buffer[8];
bool running = true;

BLE BT(&console, &hal, &state, &ioConfig, &sysConfig, &relayManager, payload);

void setup(){
  delay(1000);

  console.setVerboseLogging(true);
  configureConsole();

  state.init(TEMP_SNSR_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);
  configureFileSystem();  
  ioConfig.load();
  sysConfig.load();

  initPins();
  configureI2C();
  
  adc.setBankEnabled(1, true);
  adc.setBankEnabled(2, true);
  adc.setup(&ioConfig);
  
  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Relay Board" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), "TMPS-001");

  wifiMgr.setup();  
}

void loop(){  
  adc.loop(); 

  console.setVerboseLogging(true);
  adc.debugPrint();
  wifiMgr.loop();
  BT.update();
  delay(1000);
}