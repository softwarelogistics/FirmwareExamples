/* #define PROD_BRD_V1 */
#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>

#define WIFI_MQTT_EXAMPLE_SKU "WIFI_MQTT_EXAMPLE"
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
  sysConfig.WiFiSSID = "";
  sysConfig.WiFiPWD = "";

  state.init(WIFI_MQTT_EXAMPLE_SKU, FIRMWARE_VERSION, "v.12", "wfmq001", 010);
}

void loop() {
  /* Common loop will connect if necessary */
  commonLoop();
}