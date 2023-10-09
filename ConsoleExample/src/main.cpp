#define RELAY_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "CONSOLE_MANAGER"
#define FIRMWARE_VERSION "1.0.0"

byte buffer[8];
bool running = true;

/**
 *
 * @brief Sets the status of the command. Modifies the global `return` boolean variable.
 *
 * @param cmd:   Command signaling the function wether to restart or exit the program.
 * 
 **/

void cmdCallback(String cmd){
  if (cmd == "exit"){
    running = false;
  }
  else if (cmd == "restart"){
    running = true;
  }
}

void setup(){
  initPins();

  ledManager.setup(&ioConfig);
  ledManager.beep(20);

  configureConsole();
  console.registerCallback(cmdCallback);

  for (byte idx = 0; idx < 8; ++idx){
    buffer[idx] = 0x32 + idx;
  }

  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cns001", 010);
}

int nextPrint = 0;
int idx = 0;

void loop(){
  // console.loop() should be called to look for any input coming in
  // via a serial port.
  console.loop();
  //ledManager.loop();

  if (nextPrint < millis() && running){
    nextPrint = millis() + 1000;
    console.println("Hello World => " + String(idx++));
    console.printError("Not good!");
    console.printWarning("watch out!");
    console.printByteArray("We can describe this buffer:", buffer, 8);
    console.printByteArray(buffer, 8);
    console.printByte(0x64);
    console.newline();
    console.setVerboseLogging(false);
    console.printVerbose("This will not get sent to the console.");
    console.setVerboseLogging(true);
    console.printVerbose("This will get sent to the console.");

    console.newline();
    console.newline();
  }

  // If your app gets into a state where it can't recover and you don't want to do a restart,
  // call this method continually to write output to the console.
  // console.repeatFatalError("unrocoverable error.");
}