#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "GPRS_DEMO"
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
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cell001", 010);

  sysConfig.WiFiEnabled = false;
  sysConfig.CellEnabled = true;
  sysConfig.Commissioned = true;
  sysConfig.SrvrHostName = "mqttdev.nuviot.com";
  sysConfig.SrvrType = "mqtt";
  sysConfig.SrvrUID = "kevinw";
  sysConfig.SrvrPWD = "Test1234";
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

    mqttPublish("hithere","testing");
  }
}