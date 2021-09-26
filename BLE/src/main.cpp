#define RELAY_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_REVISION "*"
#define EXAMPLE_SKU "BLE - GATT Example"

BLE BT(&console, &hal, &state, &ioConfig, &sysConfig, &relayManager);

bool running = true;

void cmdCallback(String cmd)
{
    console.println("Receive CMD: " + cmd);
    cmd.trim();

    if (cmd == "exit")
    {
        running = false;
        wifiMQTT.disconnect();
        BT.stop();
    }
    else if (cmd == "run")
    {
        running = true;
    }
}

void setup()
{
    delay(1000);

    initPins();
    configureConsole();
    console.registerCallback(cmdCallback);

    configureFileSystem();
    loadConfigurations();

    sysConfig.SendUpdateRate = 1000;

    welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
    state.init(EXAMPLE_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "BLE001", 010);

    BT.begin("NuvIoT - BLE Sample", "123456790");
}

long nextConsoleUpdate = 0;
bool lastIsConnected = false;
void loop()
{    
    console.loop();
    BT.update();

    if (nextConsoleUpdate < millis())
    {
        if (running)
        {            
            nextConsoleUpdate = millis() + 1000;
            BT.refreshCharacteristics();
        }
        if(BT.getIsConnected())
            console.println("BLE Is Connected");
        else
            console.println("BLE Is Disconnected");
    }
}
