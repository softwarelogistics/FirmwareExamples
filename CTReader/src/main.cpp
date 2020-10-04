#include <Arduino.h>

#define GPIO_BRD_V2

#include <Objects.h>

#define VERSION "1.0.0"
#define FW_SKU "CT Example"
#define CONSOLE_BAUD_RATE 115200

void setup() {
  ioConfig.ADC1Config = ADC_CONFIG_CT;
  ioConfig.ADC1Name = "ct1";
  ioConfig.ADC1Label = "Current Tranformer 1";
  ioConfig.ADC1Scaler = 100.0;

  consoleSerial.begin(CONSOLE_BAUD_RATE, SERIAL_8N1);
  configPins.init(BOARD_CONFIG);

  console.setVerboseLogging(true);
  console.enableBTOut(false);

  configureFileSystem();
  configureI2C();

  ioConfig.load();
  sysConfig.load();

  initDisplay();

  welcome(FW_SKU, VERSION);

  powerSensor.setup(&ioConfig);
}

void loop() {
  powerSensor.loop();
  powerSensor.debugPrint();
  //float amps = powerSensor.readAmps(0);
  //console.println("CT1: " + String(amps) + "amps;");
}