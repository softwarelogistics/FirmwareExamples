#define GPIO_BRD_V2

#include <Arduino.h>
#include <NuvIoT.h>

#define VERSION "1.0.0"
#define FW_SKU "CT Example"
#define CONSOLE_BAUD_RATE 115200

void setup() {
  ioConfig.ADC1Config = ADC_CONFIG_CT;
  ioConfig.ADC1Name = "ct1";
  ioConfig.ADC1Scaler = 33.0;

  configureConsole();
  configPins.init(BOARD_CONFIG);

  configureFileSystem();
  configureI2C();

  //loadConfigurations();
  initDisplay(FW_SKU, VERSION);

  welcome(FW_SKU, VERSION);

  powerSensor.setup(&ioConfig);
}

void loop() {
  powerSensor.loop();
  powerSensor.debugPrint();
  //float amps = powerSensor.readAmps(0);
  //console.println("CT1: " + String(amps) + "amps;");
}