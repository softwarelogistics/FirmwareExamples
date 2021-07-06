#include <Arduino.h>

#define RELAY_BRD_V1

#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "GPIO - Inputs Example"

void setup() {
  ioConfig.GPIO1Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO1Name = "gpio1";
  ioConfig.GPIO1Label = "Analog to Digital Converter 1";
  ioConfig.GPIO1Scaler = 1.0;

  ioConfig.GPIO2Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO2Name = "gpio2";
  ioConfig.GPIO2Label = "Analog to Digital Converter 2";
  ioConfig.GPIO2Scaler = 1.0;

  ioConfig.GPIO3Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO3Name = "gpio3";
  ioConfig.GPIO3Label = "Analog to Digital Converter 3";
  ioConfig.GPIO3Scaler = 1.0;

  ioConfig.GPIO4Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO4Name = "gpio4";
  ioConfig.GPIO4Label = "Analog to Digital Converter 4";
  ioConfig.GPIO4Scaler = 1.0;

  ioConfig.GPIO5Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO5Name = "gpio5";
  ioConfig.GPIO5Label = "Analog to Digital Converter 5";
  ioConfig.GPIO5Scaler = 1.0;

  ioConfig.GPIO6Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO6Name = "gpio6";
  ioConfig.GPIO6Label = "Analog to Digital Converter 6";
  ioConfig.GPIO6Scaler = 1.0;

  ioConfig.GPIO7Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO7Name = "gpio7";
  ioConfig.GPIO7Label = "Analog to Digital Converter 7";
  ioConfig.GPIO7Scaler = 1.0;

  ioConfig.GPIO8Config = GPIO_CONFIG_INPUT;
  ioConfig.GPIO8Name = "gpio8";
  ioConfig.GPIO8Label = "Analog to Digital Converter 8";
  ioConfig.GPIO8Scaler = 1.0;

  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "gpio001", 010);
  initPins();

  onOffDetector.setup(&ioConfig);
}

void loop() {
  console.setVerboseLogging(true);
  onOffDetector.loop();
  onOffDetector.debugPrint();
  console.println("--");
  commonLoop();
  delay(100);
}