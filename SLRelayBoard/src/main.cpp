#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#include <Wire.h>

#define TEMP_SNSR_SKU "RLY010"
#define FIRMWARE_VERSION "0.6.1"
#define HARDWARE_REVISION "5.0"

byte buffer[8];
bool running = true;

void setup(){
  delay(1000);

  configureConsole();

  state.init(TEMP_SNSR_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);
  configureFileSystem();  
  ioConfig.load();
  sysConfig.load();

  initPins();
  writeConfigPins();
  configureI2C();
  
  adc.setBankEnabled(1, true);
  adc.setBankEnabled(2, true);
  adc.setup(&ioConfig);
  pulseCounter.setup(&ioConfig);
  onOffDetector.setup(&ioConfig);

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);
  ledManager.setErrFlashRate(5);
  
  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Relay Board" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), "RELAY-005");

  wifiMgr.setup();  

  if(sysConfig.CellEnabled)
    configureModem();

  connect();
  console.println("setup=completed;");
}

void loop(){  
  commonLoop();
}