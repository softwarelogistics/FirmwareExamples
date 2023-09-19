#define GPIO_BRD_V2

#include <Arduino.h>
#include <NuvIoT.h>

#define VERSION "1.0.0"
#define FIRMWARE_SKU "TMP_PROB_EX"
#define CONSOLE_BAUD_RATE 115200

void setup() {
  ioConfig.GPIO2Config = GPIO_CONFIG_DBS18;
  ioConfig.GPIO2Name = "temp";
  ioConfig.GPIO2Label = "BBS18 Temperature";

  configureConsole();
  
  welcome(FIRMWARE_SKU, VERSION);
  initPins();

  welcome(FIRMWARE_SKU, VERSION);

  configPins.init(BOARD_CONFIG);

  probes.setup(&ioConfig);
}

void loop() {
  probes.loop();
  probes.debugPrint();
}