#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "LED_MANAGER"
#define FIRMWARE_VERSION "1.0.1"
#define HARDWARE_REVISION "*"

int appBeepRate = 1;
int appOnlineRate = 1;
int appErrRate = 1;

void cmdCallback(String cmd)
{
  cmd.trim();
  console.println("Received command: [" + cmd + "]");

  if (cmd == "e")
  {   
    ledManager.setErrFlashRate(appErrRate);
    console.println("Setting Error LED Flash Rate: " + String(appErrRate));
    appErrRate++;
    if (appErrRate == 10)
      appErrRate = -1;
  }
  else if (cmd == "b")
  {    
    ledManager.setBeepRate(appBeepRate);
    console.println("Setting Beeper Rate: " + String(appBeepRate));    
    appBeepRate++;
    if (appBeepRate == 10)
      appBeepRate = -1;
  }
  else if (cmd == "o")
  {    
    console.println("Setting Online Rate: " + String(appOnlineRate));
    ledManager.setOnlineFlashRate(appOnlineRate);
    appOnlineRate++;
    if (appOnlineRate == 10)
      appOnlineRate = -1;
  }
  else if(cmd == "1"){
    console.println("Turning on beeper for 0.25 seconds");
    ledManager.beep(250);
  }
  else if(cmd == "2"){
    console.println("Turning on beeper for 2.5 seconds");
    ledManager.beep(2500);
  }
}

void setup()
{
  configureConsole();
  initPins();

  ledManager.setup(&ioConfig);

  /* Set flash rate equal to 1/10th of a second */
  ledManager.setErrFlashRate(appErrRate);
  ledManager.setOnlineFlashRate(appOnlineRate);
  ledManager.setBeepRate(appBeepRate);
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  
  console.registerCallback(cmdCallback);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "led001", 010);
}

void loop()
{
  console.loop();
  ledManager.loop();
}