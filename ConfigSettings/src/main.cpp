#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "CONFIG_SETTINGS"
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

    loadConfigurations();

    configureConsole();
    console.registerCallback(cmdCallback);

    for (byte idx = 0; idx < 8; ++idx)
    {
        buffer[idx] = 0x32 + idx;
    }

    sysConfig.setDefaults();
    sysConfig.DeviceId = "DEV001";
    BT.begin("NuvIoT - Config Settings Demo", "422700856E40445687BE6B41D25CBFDE");

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
        console.println("Commissioned :" + String(sysConfig.Commissioned));
        console.println("Device Id    :" + String(sysConfig.DeviceId));
        console.println("Device Access:" + String(sysConfig.DeviceAccessKey));
        console.println("Verbose Log  :" + String(sysConfig.VerboseLogging));
        console.newline();

        console.println("WiFi Enabled :" + String(sysConfig.WiFiEnabled));
        console.println("WiFi SSID    :" + String(sysConfig.WiFiSSID));
        console.println("WiFi PWD     :" + String(sysConfig.WiFiPWD));
        console.println("Cell Enabled :" + String(sysConfig.CellEnabled));
        console.println("Modem Baud   :" + String(sysConfig.GPRSModemBaudRate));
        console.println("Send Rate    :" + String(sysConfig.SendUpdateRate));
        console.newline();

        console.println("Ping Rate    :" + String(sysConfig.PingRate));
        console.println("Srvr TLS     :" + String(sysConfig.TLS));
        console.println("Srvr Host    :" + String(sysConfig.SrvrHostName));
        console.println("Srvr Anonyms :" + String(sysConfig.Anonymous));
        console.println("Srvr UID     :" + String(sysConfig.SrvrUID));
        console.println("Srvr PWD     :" + String(sysConfig.SrvrPWD));
        console.newline();
        

        console.println("GPS Enabled  :" + String(sysConfig.GPSEnabled));
        console.println("GPS Update Rt:" + String(sysConfig.GPSUpdateRate));
        console.println("Cell Enabled :" + String(sysConfig.CellEnabled));
        console.newline();

        
    }

    // If your app gets into a state where it can't recover and you don't want to do a restart,
    // call this method continually to write output to the console.
    // console.repeatFatalError("unrocoverable error.");
}