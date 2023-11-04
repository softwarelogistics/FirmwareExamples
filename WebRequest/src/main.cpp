/* #define PROD_BRD_V1 */
#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "WEB001"
#define FIRMWARE_VERSION "0.3.0"

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
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "dfu001", 010);

  sysConfig.WiFiEnabled = false;
  sysConfig.CellEnabled = true;
  sysConfig.Commissioned = true;
  sysConfig.SrvrType = "http";
  sysConfig.SrvrHostName = "pt1.seawolf.iothost.net";
  sysConfig.PingRate = 5;
  sysConfig.SendUpdateRate = 2500;
  sysConfig.DeviceId = "floyd";

  connect();
  console.setVerboseLogging(false);
}

long nextPrint = 0;

void loop()
{
  commonLoop();
  if (nextPrint < millis() && running)
  {
    nextPrint = millis() + sysConfig.SendUpdateRate;
    //httpGet("http://" + sysConfig.SrvrHostName + "/sensor/" + sysConfig.DeviceId + "/bilge/data");
    httpPost("http://" + sysConfig.SrvrHostName + "/sensor/" + sysConfig.DeviceId + "/bilge/data", "1,1,0,0,1,1,0");
    console.newline();
  }
}