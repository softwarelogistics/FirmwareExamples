/* #define PROD_BRD_V1 */
#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>

#include <NuvIoT.h>

/*
 * This example will show how to cycle the power on the communications
 * peripheral.  
 */

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_REVISION "*"
#define EXAMPLE_SKU "Modem Power"

void cmdCallback(String cmd)
{
    cmd.trim();
    console.println("Received command: [" + cmd + "]");

    if (cmd == "on")
    {
        modem.hardwarePowerOn();
        digitalWrite(18, LOW);
    }
    else if (cmd == "off")
    {
        modem.hardwarePowerOff();
        digitalWrite(18, HIGH);
    }
    else if (cmd == "reset")
    {
        modem.hardwareReset();
    }
    else if (cmd == "soff")
    {
        modem.softwarePowerOff();
    }
    else
    {
        console.println("Unknown commnd: [" + cmd + "]");
    }
}

void setup()
{
    initPins();
    configureConsole();

    configureModem();

    console.registerCallback(cmdCallback);
    welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
    state.init(EXAMPLE_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "mdmpwr001", 010);
}

int idx = 0;

void loop()
{
    console.loop();    
    modem.isModemOnline();
    delay(500);
}