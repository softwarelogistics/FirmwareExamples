#define PROD_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "IO_VALUES"
#define FIRMWARE_VERSION "1.0.1"

bool running = true;
double testADCValues[8];
double testTemperatureValues[8];

BLE BT(&console, &hal, &state, &ioConfig, &sysConfig, &relayManager, payload);

void cmdCallback(String cmd)
{
    console.println("Receive CMD: " + cmd);
    cmd.trim();

    if (cmd == "exit")
    {
        running = false;
    }
    else if (cmd == "restart")
    {
        running = true;
    }
    else if (cmd == "+")
    {
        running = true;
        for (int idx = 0; idx < 8; ++idx)
        {
            testADCValues[idx] *= 1.2;
            testTemperatureValues[idx] *= 1.2;
        }
    }
    else if (cmd == "-")
    {
        running = true;
        for (int idx = 0; idx < 8; ++idx)
        {
            testADCValues[idx] *= 0.8;
            testTemperatureValues[idx] *= 0.8;
        }
    }
    else{
        console.println("Unknown Command [" + cmd + "]");
    }
}

void setup()
{
    initPins();
    configureConsole();
    console.registerCallback(cmdCallback);

    sysConfig.setDefaults();
    BT.begin("NuvIoT - IO Values", "422700856E40445687BE6B41D25CBFDE");

    welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

    state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cfg001", 010);

    ioConfig.setDefaults();

    ioConfig.ADC1Config = ADC_CONFIG_ADC;
    ioConfig.ADC1Calibration = 1354.1;
    ioConfig.ADC1Zero = 800;
    ioConfig.ADC3Config = ADC_CONFIG_ADC;
    ioConfig.ADC3Scaler = 15;
    ioConfig.ADC4Config = ADC_CONFIG_ADC;
    ioConfig.ADC4Zero = 1.2;

    adc.configure(&ioConfig);

    /* This values will simulate values
       in volts out of 5 possible volts 
       coming from the ADC */

    testADCValues[0] = 1.2;
    testADCValues[2] = 1.5;
    testADCValues[3] = 1.7;

    ioConfig.GPIO3Config = GPIO_CONFIG_DBS18;
    ioConfig.GPIO5Config = GPIO_CONFIG_DBS18;
    ioConfig.GPIO7Config = GPIO_CONFIG_DBS18;
    probes.configure(&ioConfig);

    testTemperatureValues[2] = 89.9;
    testTemperatureValues[4] = 54.3;
    testTemperatureValues[6] = 100.3;
}

int nextPrint = 0;

void loop()
{
    console.loop();
    BT.update();

    if (nextPrint < millis() && running)
    {
        /* We are passing in an array of 
           voltages to simulate what we 
           would normally read from the ADC */
        adc.loop(testADCValues);
        probes.loop(testTemperatureValues);

        console.setVerboseLogging(true);
        /* IO Values consists of an array of 
           16 values are that the outputs of 
           the configured sensor */
        ioValues.debugPrint();
        console.setVerboseLogging(false);

        nextPrint = millis() + 1000;

        if (!BT.getIsConnected())
        {
            console.println("ble:disconnected;");
        }    
    }
}