#define RELAY_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "ADC Example"

void setup() {
  // @todo: make function to abstract this process 
  ioConfig.ADC1Config = ADC_CONFIG_ADC; 
  ioConfig.ADC1Name = "adc1";
  ioConfig.ADC1Scaler = 2.0;
  ioConfig.ADC1Calibration = 1.0;

  ioConfig.ADC2Config = ADC_CONFIG_ADC;
  ioConfig.ADC2Name = "adc2";
  ioConfig.ADC2Scaler = 2.0;
  ioConfig.ADC2Calibration = 1.0;

  ioConfig.ADC3Config = ADC_CONFIG_ADC;
  ioConfig.ADC3Name = "adc3";
  ioConfig.ADC3Scaler = 2.0;
  ioConfig.ADC3Calibration = 1.0;

  ioConfig.ADC4Config = ADC_CONFIG_ADC;
  ioConfig.ADC4Name = "adc4";  
  ioConfig.ADC4Scaler = 2.0;
  ioConfig.ADC4Calibration = 1.0;

  ioConfig.ADC5Config = ADC_CONFIG_ADC;
  ioConfig.ADC5Name = "adc5";
  ioConfig.ADC5Scaler = 2.0;
  ioConfig.ADC5Calibration = 1.0;

  ioConfig.ADC6Config = ADC_CONFIG_ADC;
  ioConfig.ADC6Name = "adc6";
  ioConfig.ADC6Scaler = 2.0;
  ioConfig.ADC6Calibration = 1.0;

  ioConfig.ADC7Config = ADC_CONFIG_ADC;
  ioConfig.ADC7Name = "adc7";
  ioConfig.ADC7Scaler = 2.0;
  ioConfig.ADC7Calibration =  1.0;

  ioConfig.ADC8Config = ADC_CONFIG_ADC;
  ioConfig.ADC8Name = "adc8";
  ioConfig.ADC8Scaler = 2.0;
  ioConfig.ADC8Calibration = 1.0;

  initPins();

  // @todo: display error description for i2c address fail
  configureI2C();
  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  
  configureFileSystem();

  adc.setBankEnabled(1, true);
  adc.setBankEnabled(2, true);

  adc.setup(&ioConfig);
}

void loop() {
  console.setVerboseLogging(true);
  adc.loop(); 
  adc.debugPrint();
  console.println("--");
  delay(2500);
}