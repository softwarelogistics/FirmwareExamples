#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "IO_VALUES"
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

    state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "io001", 010);
}

int nextPrint = 0;

void loop()
{
    console.loop();
    
    if (nextPrint < millis() && running)
    {
        BT.update(); 

        nextPrint = millis() + 1000;

        if(!BT.getIsConnected()) {
            console.println("ble:disconnected;");
        }
    }
}