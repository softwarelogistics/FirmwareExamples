#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>

#define TEMP_SNSR_SKU "Temperature Sensor"
#define FIRMWARE_VERSION "0.5.0"
#define HARDWARE_REVISION "3.0"

byte buffer[8];
bool running = true;

BLE BT(&console, &hal, &state, &ioConfig, &sysConfig, &relayManager, payload);

void cmdCallback(String cmd){
    console.println("Receive CMD: " + cmd);

    if (cmd == "exit")
    {
        running = false;
        wifiMQTT.disconnect();
        BT.stop();
    }
    else if (cmd == "run")
    {
        running = true;
    }
}

void setup()
{
  delay(1000);

  configureFileSystem();

  ioConfig.load();
  sysConfig.load();
  sysConfig.WiFiEnabled = true;
  state.init(TEMP_SNSR_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);
  initPins();

  //ledManager.setup(&ioConfig);

  configureConsole();
  console.registerCallback(handleConsoleCommand);
  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Temp Sensor" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), "TMPS-001");

  wifiMgr.setup();  
}

int nextPrint = 0;
int idx = 0;

void loop()
{
  console.loop();
  ledManager.loop();  
  wifiMgr.loop();
  BT.update();
}