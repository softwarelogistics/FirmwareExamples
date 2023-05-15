#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>
#include <HX711.h>

#define SKU "HX711_RDR"
#define FIRMWARE_VERSION "0.2.0"
#define HARDWARE_REVISION "5"

#define SAMPLE_COUNT 2
#define SAMPLE_SIZE 60

float sampleBuffer[SAMPLE_COUNT][SAMPLE_SIZE];

bool running = true;

HX711 loadCell;

#define LOAD_CELL_SCK 25
#define LOAD_CALL_DOUT 26

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
  delay(1000);

  configureFileSystem();

  loadCell.begin(LOAD_CALL_DOUT, LOAD_CELL_SCK);
  loadCell.set_scale(1);
  loadCell.set_offset(0);

  ioConfig.load();
  sysConfig.load();
  sysConfig.SrvrType = "mqtt";
  sysConfig.Port = 1883;
  sysConfig.SrvrHostName = "paw-mqtt.iothost.net";
  //sysConfig.SrvrUID = XXXXX;
  //sysConfig.SrvrPWD = XXXXX;
  sysConfig.WiFiEnabled = false;
  sysConfig.CellEnabled = true;
  sysConfig.Commissioned = false;
  sysConfig.DeviceId = "mg2";
  sysConfig.SendUpdateRate = 60;
  state.init(SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "wmd", 010);

  initPins();

  configureConsole();
  writeConfigPins();

  configureModem();

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Weight Sensor" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), SKU);

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);
  ledManager.setErrFlashRate(0);

  sysConfig.print();

  console.setVerboseLogging(false);

  if (sysConfig.Commissioned && sysConfig.CellEnabled)
  {
    connect();
  }

  welcome(SKU, FIRMWARE_VERSION);

  
  //  connect();
  // put your setup code here, to run once:
}

int nextPrint = 0;
int idx = 0;

int nextCapture = 0;
byte msgBuffer[4096];

#define SAMPLE_INTERVAL 1.0

void loop()
{
  commonLoop();

  if (nextCapture < millis())
  {
    nextCapture = millis() + 1000;

    loadCell.set_offset(ioConfig.GPIO1Zero);
    loadCell.set_gain(64);
    sampleBuffer[0][idx] = loadCell.get_value() - 1360000;

    loadCell.set_offset(ioConfig.GPIO2Zero);
    loadCell.set_gain(32);
    sampleBuffer[1][idx] = loadCell.get_value();

    console.print(String(idx) + ". Weight ");
    console.print(String(sampleBuffer[0][idx]));
    console.print(", ");
    console.print(String(sampleBuffer[1][idx]));
    console.println(";");
    idx++;


    if (idx >= SAMPLE_SIZE)
    {
      for(int sensor_index = 0; sensor_index < 2; ++sensor_index)
      {
        idx = 0;
        if(sysConfig.Commissioned){
        // Let the server add in the time stamp.
        msgBuffer[0] = 0;
        msgBuffer[1] = 0;
        msgBuffer[2] = 0;
        msgBuffer[3] = 0;

        msgBuffer[4] = sensor_index+1;
        msgBuffer[5] = 0;

        msgBuffer[6] = (SAMPLE_SIZE & 0xFF);
        msgBuffer[7] = (SAMPLE_SIZE >> 8) & 0xFF;

        uint16_t scaledInterval = (uint16_t)(SAMPLE_INTERVAL * 10.0f);
        msgBuffer[8] = (scaledInterval & 0xFF);
        msgBuffer[9] = (scaledInterval >> 8) & 0xFF;

        for (uint16_t pointIndex = 0; pointIndex < SAMPLE_SIZE; pointIndex++)
        {
          int16_t scaledPoint = (int16_t)(sampleBuffer[sensor_index][pointIndex] * 100.0f);
          scaledPoint = pointIndex * 10;
          msgBuffer[10 + (pointIndex * 2)] = (scaledPoint & 0xFF);
          msgBuffer[10 + (pointIndex * 2) + 1] = ((scaledPoint >> 8) & 0xFF);
        }

        long start = millis();
        uint16_t buffSize = (SAMPLE_SIZE * 2) + 10;

          mqttPublish("infmon/" + sysConfig.DeviceId + "/sensor/" + String(sensor_index + 1) + "/weight", msgBuffer, buffSize, QOS0);
          delay(1000);
        }

      }
    }
  }
}