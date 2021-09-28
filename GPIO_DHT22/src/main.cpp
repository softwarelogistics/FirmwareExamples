#include <Arduino.h>

#define RELAY_BRD_V1

#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "GPIO - DHT22"

void setup() {
  ioConfig.GPIO1Config = GPIO_CONFIG_DHT22;
  ioConfig.GPIO1Name = "dht22.1";

  ioConfig.GPIO2Config = GPIO_CONFIG_DBS18;
  ioConfig.GPIO2Name = "ds18b20.2";
  
  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0.0", "gpio002", 010);
  initPins();

  probes.setup(&ioConfig);
}

void loop() {
  console.setVerboseLogging(true);
  probes.loop();
  probes.debugPrint();
  console.println("--");
  commonLoop();
  delay(1000);
}