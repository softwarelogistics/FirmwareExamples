#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "LED_MANAGER"
#define FIRMWARE_VERSION "1.0.0"

void setup() {
  configureI2C();
  configureConsole();
  initPins();

  ledManager.setup(&ioConfig);

  /* Set flash rate equal to 1/10th of a second */
  ledManager.setErrFlashRate(1);
  ledManager.setOnlineFlashRate(10);
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "led001", 010);
}

void loop() {
  /* nothing to do, led flashing is controlled by IRQs */
}