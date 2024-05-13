/* #define RELAY_BRD_V1 */

#define CAN_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.1"
#define EXAMPLE_SKU "ESP32 Onboard ADC Example"

void setup() {
  ioConfig.ADC1Config = ADC_CONFIG_VOLTS;
  ioConfig.ADC1Name = "adc1";
  ioConfig.ADC1Scaler = 2.0;
  ioConfig.ADC1Calibration = 1.0;
  ioConfig.ADC1Zero = -0.12;

  ioConfig.ADC2Config = ADC_CONFIG_VOLTS;
  ioConfig.ADC2Name = "adc2";
  ioConfig.ADC2Scaler = 2.0;
  ioConfig.ADC2Calibration = 1.0;
  ioConfig.ADC2Zero = -0.12;

  ioConfig.ADC3Config = ADC_CONFIG_VOLTS;
  ioConfig.ADC3Name = "adc3";
  ioConfig.ADC3Scaler = 2.0;
  ioConfig.ADC3Calibration = 1.0;
  ioConfig.ADC3Zero = -0.12;

  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "adc002", "1", 1);
  initPins();

  adc.setup(&ioConfig);
}

uint32_t reportInterval = 0;

void loop() {
  commonLoop();
  
  if(millis() > reportInterval) {
    console.setVerboseLogging(true);
    adc.debugPrint();
    console.println("--");
    console.setVerboseLogging(false);
    reportInterval = millis() + 1000;
  }
}