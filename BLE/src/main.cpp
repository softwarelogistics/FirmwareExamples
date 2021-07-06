#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>

#include <NuvIoTMin.h>

#include "BLE.h"

#define RELAY_BRD_V1

//#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_REVISION "*"
#define EXAMPLE_SKU "BLE - GATT Example"

#define SVC_UUID_SYS "d804b639-6ce7-4e80-9f8a-ce0f699085eb"
#define CHAR_UUID_SYS_STATE "d804b639-6ce7-4e81-9f8a-ce0f699085eb"
#define CHAR_UUID_SYS_REBOOT "d804b639-6ce7-4e82-9f8a-ce0f699085eb"
#define CHAR_UUID_SYS_RELOAD "d804b639-6ce7-4e83-9f8a-ce0f699085eb"

String getWiFiStatus(int status)
{
    String statusMsg = "";

    switch (status)
    {
    case WL_IDLE_STATUS:
        statusMsg = "Idle";
        break;
    case WL_NO_SHIELD:
        statusMsg = "No Shield";
        break;
    case WL_NO_SSID_AVAIL:
        statusMsg = "No SSID Available";
        break;
    case WL_SCAN_COMPLETED:
        statusMsg = "Scan Completed";
        break;
    case WL_CONNECTED:
        statusMsg = "Connected";
        break;
    case WL_CONNECT_FAILED:
        statusMsg = "Connection Failed";
        break;
    case WL_CONNECTION_LOST:
        statusMsg = "Connection Lost";
        break;
    case WL_DISCONNECTED:
        statusMsg = "Disconnected";
        break;
    }

    return statusMsg;
}

BLE BT(&console, &hal, &state, &ioConfig, &sysConfig, &relayManager);

//HardwareSerial ble(0);

// similar to what we want MQTT and WiFi
// https://github.com/1technophile/OpenMQTTGateway/issues/563

void setup() {

  //console.println("START " + String(ESP.getFreeHeap()));
  delay(1000);

  configureFileSystem();
  configureI2C();
  //configureConsole();

  //console.println("START 1 " + String(ESP.getFreeHeap()));

  ioConfig.load();
  sysConfig.load();
  
  //console.println(sysConfig.DeviceId);

  //state.init(EXAMPLE_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);

  //console.println("START 4 " + String(ESP.getFreeHeap()));

  consoleSerial.begin(115200, SERIAL_8N1);
  console.println("");
  console.println("RESTARTING");

  BT.begin("NuvIoT - BLE Sample");

  wifiMgr.setup();
  wifiMgr.connect(true); 
}

int idx = 0;

void loop() {
    delay(500);
    //commonLoop();
    console.println("WIFI START " + getWiFiStatus(WiFi.status()));
    console.println("FREE RAM " + String(ESP.getFreeHeap()));  
    if(WiFi.status() == WL_CONNECTED) {
      //wifiMQTT.connect();
      //client.WifiConnect(true);
    }

    delay(500);
    BT.update(); 
}