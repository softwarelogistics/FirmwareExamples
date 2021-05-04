#include <Arduino.h>

#define PRD_BRD_V2

#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "ADC Example"

void setup() {
  ioConfig.ADC4Config = ADC_CONFIG_ADC;
  ioConfig.ADC4Name = "adc1";
  ioConfig.ADC4Label = "Analog to Digital Converter 1";
  ioConfig.ADC4Scaler = 1.0;

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
  //float volts = adc.getVoltage(4);
  //console.println("ADC1: " + String(volts) + " volts;");
  delay(100);
}