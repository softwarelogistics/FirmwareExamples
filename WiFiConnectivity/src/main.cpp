#include <Arduino.h>
#include <NuvIoT.h>

#define WIFI_EXAMPLE_SKU "WIFIEXAMPLE"
#define FIRMWARE_VERSION "1.0.0"


void setup() {
  Serial.begin(115200);

  configureI2C();
  configureConsole();
  initPins();

  ledManager.setup(&ioConfig);
  
  sysConfig.DeviceId = "?";
  sysConfig.Commissioned = true;
  sysConfig.WiFiEnabled = true;
  sysConfig.WiFiSSID = "[YOURSSID]";
  sysConfig.WiFiPWD = "[YOURPWD]";

  state.init(WIFI_EXAMPLE_SKU, FIRMWARE_VERSION, "pcl001", "DONTCARE", 010);
}

void loop() {
  /* Common loop will connect if necessary */
  commonLoop();
}