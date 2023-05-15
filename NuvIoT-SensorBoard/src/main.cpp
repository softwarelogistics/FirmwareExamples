#define DHT_DEBUG

#define TEMP_SNSR_BOARD_V3

#include <TemperatureProbes.h>
#include <Arduino.h>
#include <NuvIoT.h>
#include <NuvIoTSensors.h>

#define TEMP_SNSR_SKU "RSB-01"
#define FIRMWARE_VERSION "0.9.1"
#define HARDWARE_REVISION "4.0"

#define BATT_SENSE_PIN 34
#define TEMP_SENSE_PIN 35

#define IO1_PIN 25
#define IO2_PIN 26

byte buffer[8];
bool running = true;


void setup()
{
  delay(1000);

  configureFileSystem();

  ioConfig.load();
  sysConfig.load();
  sysConfig.SendUpdateRate = 1000;

  state.init(TEMP_SNSR_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);
  
  initPins();

  configureConsole();
  

  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  determineSensorConfiguration(&ioConfig, &console, IO1_PIN, IO2_PIN);
  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Temp Sensor" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), TEMP_SNSR_SKU);

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);  
  ledManager.setErrFlashRate(0);

  if (sysConfig.Commissioned && sysConfig.CellEnabled)
  {
    connect();
    configureModem();
  }

  if(sysConfig.Commissioned && sysConfig.WiFiEnabled)
  {
    wifiMgr.setup();  
  }

  probes.configure(&ioConfig);
}

void loop(){
  commonLoop();

  BT.update();

  sendIOValues();

  console.setVerboseLogging(true);
  probes.debugPrint();
  delay(1000);
}