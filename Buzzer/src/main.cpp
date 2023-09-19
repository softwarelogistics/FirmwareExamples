#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "LED_MANAGER"
#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_REVISION "*"

int onlineRate = 0;
int errRate = 0;

void cmdCallback(String cmd)
{
  cmd.trim();
  console.println("Received command: [" + cmd + "]");

  if (cmd == "e")
  {
    ledManager.setErrFlashRate(errFlashRate);
    console.println("Setting Error LED Flash Rate: " + String(errFlashRate));
    if (errFlashRate == 10)
      errFlashRate = 0;
  }
  else if (cmd == "b")
  {
    console.println("Setting Beeper Rate: " + String(beepRate));
    ledManager.setBeepRate(beepRate);
    if (beepRate == 10)
      beepRate = 0;
  }
  else if (cmd == "o")
  {
    console.println("Setting Online Rate: " + String(onlineRate));
    ledManager.setOnlineFlashRate(onlineRate);
    if (onlineRate == 10)
      onlineRate = 0;
  }
}

void setup()
{
  configureConsole();
  initPins();

  ledManager.setup(&ioConfig);

  /* Set flash rate equal to 1/10th of a second */
  ledManager.setErrFlashRate(1);
  ledManager.setOnlineFlashRate(10);
  ledManager.setBeepRate(5);
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "led001", 010);
}

void loop()
{
  console.loop();
}