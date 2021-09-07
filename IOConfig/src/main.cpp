#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "IO_CONFIG"
#define FIRMWARE_VERSION "1.0.1"

byte buffer[8];
bool running = true;

BLE BT(&console, &hal, &state, &ioConfig, &sysConfig, &relayManager);

void cmdCallback(String cmd)
{
    if (cmd == "exit")
    {
        running = false;
    }
    else if (cmd == "restart")
    {
        running = true;
    }
}

void setup()
{
    initPins();
    configureFileSystem();
    configureConsole();
    console.setVerboseLogging(true);
    loadConfigurations();
    console.setVerboseLogging(false);
    
    console.registerCallback(cmdCallback);

    for (byte idx = 0; idx < 8; ++idx)
    {
        buffer[idx] = 0x32 + idx;
    }

    sysConfig.setDefaults();
    sysConfig.DeviceId = "DEV001";
    BT.begin("NuvIoT - IO Configuration", "422700856E40445687BE6B41D25CBFDE");

    welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

    state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cfg001", 010);
}

int nextPrint = 0;
int idx = 0;

void loop()
{
    // console.loop() should be called to look for any input coming in
    // via a serial port.
    console.loop();
    BT.update();    

    if (nextPrint < millis() && running)
    {
        nextPrint = millis() + 1000;
        console.println("hi " + String(idx++));
        console.newline();
    }

    // If your app gets into a state where it can't recover and you don't want to do a restart,
    // call this method continually to write output to the console.
    // console.repeatFatalError("unrocoverable error.");
}