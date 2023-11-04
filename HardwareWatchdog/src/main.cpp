#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_REVISION "*"
#define EXAMPLE_SKU "Hardware Watchdog"

#define WATCHDOG_TIMEOUT_SECONDS 5

long watchdogExpires = 0;

void cmdCallback(String cmd)
{
    cmd.trim();

    if (cmd == "f")
    {
        watchdogExpires = millis() + (WATCHDOG_TIMEOUT_SECONDS * 1000);
        console.println("Fed Watchdog");
        hal.feedHWWatchdog();
    }

    if(cmd == "r")
    {
        hal.restart();
    }
}

void setup()
{
    initPins();
    configureConsole();
    
    welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

    console.registerCallback(cmdCallback);
    state.init(EXAMPLE_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "hwwd001", 010);
    console.println("App has restarted, reason: " + hal.resetReasonMessage());

    hal.enableHWWatchdog(WATCHDOG_TIMEOUT_SECONDS);
    watchdogExpires = millis() + (WATCHDOG_TIMEOUT_SECONDS * 1000);
}

long nextConsoleUpdate = 0;

void loop()
{
    console.loop();
    if(nextConsoleUpdate < millis())
    {
        nextConsoleUpdate = millis() + 250;
        console.println("app is alive, watch dog fires in " + String(watchdogExpires - millis()) + "ms");
    }
}