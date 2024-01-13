/* #define RELAY_BRD_V1 */

#define CAN_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "Pulse Counter Example"

void setup() {
  ioConfig.GPIO1Config = GPIO_CONFIG_PULSE_COUNTER;
  ioConfig.GPIO1Name = "gpio1";
  ioConfig.GPIO1Scaler = 1.0;

  ioConfig.GPIO2Config = GPIO_CONFIG_PULSE_COUNTER;
  ioConfig.GPIO2Name = "gpio2";
  ioConfig.GPIO2Scaler = 1.0;

  ioConfig.GPIO3Config = GPIO_CONFIG_PULSE_COUNTER;
  ioConfig.GPIO3Name = "gpio3";
  ioConfig.GPIO3Scaler = 1.0;

  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "gpio001", "1", 1);
  initPins();

  pulseCounter.setup(&ioConfig);
}

uint32_t reportInterval = 0;

void loop() {
  commonLoop();
  
  if(millis() > reportInterval) {
    console.setVerboseLogging(true);
    pulseCounter.debugPrint();
    console.println("--");
    console.setVerboseLogging(false);
    reportInterval = millis() + 1000;
  }
}