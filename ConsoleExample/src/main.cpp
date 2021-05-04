#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "CONSOLE_MANAGER"
#define FIRMWARE_VERSION "1.0.0"

byte buffer[8];

void setup() {
  configureI2C();
  configureConsole();
  initPins();

  for(byte idx = 0; idx < 8; ++idx){
    buffer[idx] = 0x32 + idx;
  }

  // Anything written to the console will automatically be sent to any active bluetooth connections
  console.enableBTOut(true);

  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  
  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "cns001", 010);
}

int nextPrint = 0;
int idx = 0;


void loop() {
  // console.loop() should be called to look for any input coming in 
  // via a serial port.
  console.loop();

  if(nextPrint < millis()) {
    nextPrint = millis() + 1000;
    console.println("Hello World" + String(idx++));
    console.printError("Not good!");
    console.printWarning("watch out!");
    console.printByteArray("We can describe this buffer:", buffer,8);
    console.printByteArray(buffer,8);
    console.printByte(0x64);
    console.newline();
    console.setVerboseLogging(false);
    console.printVerbose("This will not get sent to the console.");
    console.setVerboseLogging(true);
    console.printVerbose("This will get sent to the console.");
  }

  // If your app gets into a state where it can't recover and you don't want to do a restart, 
  // call this method continually to write output to the console.
  // console.repeatFatalError("unrocoverable error.");
}