#include <Arduino.h>

#define GPIO_BRD_V2

#include <Objects.h>

#define VERSION "1.0.0"
#define FW_SKU "ADC Example"
#define CONSOLE_BAUD_RATE 115200


void setup() {
  ioConfig.ADC4Config = ADC_CONFIG_ADC;
  ioConfig.ADC4Name = "adc1";
  ioConfig.ADC4Label = "Analog to Digital Converter 1";
  ioConfig.ADC4Scaler = 1.0;

  consoleSerial.begin(CONSOLE_BAUD_RATE, SERIAL_8N1);
  configPins.init(BOARD_CONFIG);

  console.setVerboseLogging(true);
  console.enableBTOut(false);

  configureFileSystem();
  configureI2C();

  sysConfig.load();

  initDisplay();

  welcome(FW_SKU, VERSION);

  adc.setup(&ioConfig);
}

void loop() {
  adc.loop(); 
  adc.debugPrint();
  //float volts = adc.getVoltage(4);
  //console.println("ADC1: " + String(volts) + " volts;");
  delay(100);
}