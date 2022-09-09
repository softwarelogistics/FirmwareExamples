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
uint8_t new_mac[8] = {0x10, 0x02, 0x03, 0x04, 0x05, 0x06};
esp_base_mac_addr_set(new_mac);

  delay(1000);

  console.setVerboseLogging(true);
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

  ledManager.setOnlineFlashRate(1);
  ledManager.setErrFlashRate(0);
  
  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Relay Board" : sysConfig.DeviceId);
  console.println("BT DEVICE NAME: " + btName);

  BT.begin(btName.c_str(), "RELAY-005");

  wifiMgr.setup();  
}

void loop(){  
  adc.loop(); 

  console.setVerboseLogging(true);
  adc.debugPrint();
  wifiMgr.loop();
  BT.update();
  hal.loop();
  delay(1000);
}