#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "GPS_DEMO"
#define FIRMWARE_VERSION "1.0.0"

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
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "gps001", 010);

  sysConfig.WiFiEnabled = false;
  sysConfig.CellEnabled = false;
  sysConfig.GPSEnabled = true;
  sysConfig.DeviceId = "GPSDEMO";

  console.setVerboseLogging(false);
  connect();
}

int nextPrint = 0;
int idx = 0;

void loop()
{
  console.loop();  

  if (nextPrint < millis() && running)
  {
    nextPrint = millis() + 1000;
    console.setVerboseLogging(true);
    GPSData *gpsData = readGPS();
    console.setVerboseLogging(false);
    console.println("Sat in view: " + String(gpsData->satellitesinview));

    console.newline();
  }
}