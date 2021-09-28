#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "GPRS_DEMO"
#define FIRMWARE_VERSION "1.0.0"

#define SRVR_HOST_NAME "pt1.seawolf.iothost.net";
#define SRVR_UID "seawolf";
#define SRVR_PWD "4NuvIoT!";

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
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cell001", 010);

  sysConfig.WiFiEnabled = false;
  sysConfig.CellEnabled = true;
  sysConfig.Commissioned = true;
  sysConfig.SrvrHostName = SRVR_HOST_NAME;
  sysConfig.SrvrUID = SRVR_UID;
  sysConfig.SrvrPWD = SRVR_PWD;
  sysConfig.DeviceId = "CELLDEMO";

  console.setVerboseLogging(false);
  connect();
  // put your setup code here, to run once:
}

int nextPrint = 0;
int idx = 0;

void loop()
{
  console.loop();  

  if (nextPrint < millis() && running)
  {
    nextPrint = millis() + 1000;
    console.println("Hello World => " + String(idx++));
    console.newline();
  }
}