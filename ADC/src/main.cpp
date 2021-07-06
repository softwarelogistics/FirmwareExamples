#include <Arduino.h>

#define RELAY_BRD_V1

#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "ADC Example"

void setup() {
  ioConfig.ADC1Config = ADC_CONFIG_ADC;
  ioConfig.ADC1Name = "adc1";
  ioConfig.ADC1Label = "Analog to Digital Converter 1";
  ioConfig.ADC1Scaler = 1.0;

  ioConfig.ADC2Config = ADC_CONFIG_ADC;
  ioConfig.ADC2Name = "adc2";
  ioConfig.ADC2Label = "Analog to Digital Converter 1";
  ioConfig.ADC2Scaler = 1.0;

  ioConfig.ADC3Config = ADC_CONFIG_ADC;
  ioConfig.ADC3Name = "adc3";
  ioConfig.ADC3Label = "Analog to Digital Converter 1";
  ioConfig.ADC3Scaler = 1.0;

  ioConfig.ADC4Config = ADC_CONFIG_ADC;
  ioConfig.ADC4Name = "adc4";
  ioConfig.ADC4Label = "Analog to Digital Converter 1";
  ioConfig.ADC4Scaler = 1.0;

  ioConfig.ADC5Config = ADC_CONFIG_ADC;
  ioConfig.ADC5Name = "adc5";
  ioConfig.ADC5Label = "Analog to Digital Converter 1";
  ioConfig.ADC5Scaler = 1.0;

  ioConfig.ADC6Config = ADC_CONFIG_ADC;
  ioConfig.ADC6Name = "adc6";
  ioConfig.ADC6Label = "Analog to Digital Converter 1";
  ioConfig.ADC6Scaler = 1.0;

  ioConfig.ADC7Config = ADC_CONFIG_ADC;
  ioConfig.ADC7Name = "adc7";
  ioConfig.ADC7Label = "Analog to Digital Converter 1";
  ioConfig.ADC7Scaler = 1.0;

  ioConfig.ADC8Config = ADC_CONFIG_ADC;
  ioConfig.ADC8Name = "adc8";
  ioConfig.ADC8Label = "Analog to Digital Converter 1";
  ioConfig.ADC8Scaler = 1.0;

  configureI2C();
  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  
  initPins();

  configureFileSystem();
  configureI2C();

  adc.setBankEnabled(1, true);
  adc.setBankEnabled(2, true);

  adc.setup(&ioConfig);
}

void loop() {
  console.setVerboseLogging(true);
  adc.loop(); 
  adc.debugPrint();
  console.println("--");
  delay(100);
}