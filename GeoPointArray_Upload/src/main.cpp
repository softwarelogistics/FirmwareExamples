#include <Arduino.h>
#include <NuvIoT.h>
#include "../../Common/set_secrets.h"

#define GEO_POINT_ARRAY_UPLOAD_EXAMPLE_SKU "GEO_PT_UPLOAD"
#define FIRMWARE_VERSION "1.0.0"

void setup()
{
  Serial.begin(115200);

  configureI2C();
  configureConsole();
  initPins();

  ledManager.setup(&ioConfig);

  sysConfig.Commissioned = true;
  sysConfig.WiFiEnabled = true;
  sysConfig.SrvrType = "mqtt";
  setSysConfigSecrets(&sysConfig);

  state.init(GEO_POINT_ARRAY_UPLOAD_EXAMPLE_SKU FIRMWARE_VERSION, "00", "geopt", "geopt", 010);
}

#define SAMPLE_COUNT 60
#define SAMPLE_INTERVAL 1

float sampleBuffer[SAMPLE_COUNT][2];
byte msgBuffer[10 + (SAMPLE_COUNT * (4 * 2))];

void assembleGeoPointArray()
{
  for (int idx = 0; idx < 60; ++idx)
  {
    sampleBuffer[idx][0] = 32.425 - ((float)idx / 10); // latitude
    sampleBuffer[idx][1] = 28.425 - (float)(idx / 10); // longitude
  }

  msgBuffer[0] = 0;
  msgBuffer[1] = 0;
  msgBuffer[2] = 0;
  msgBuffer[3] = 0;

  msgBuffer[4] = 1;
  msgBuffer[5] = 0;

  msgBuffer[6] = (SAMPLE_COUNT & 0xFF);
  msgBuffer[7] = (SAMPLE_COUNT >> 8) & 0xFF;

  uint16_t scaledInterval = (uint16_t)(SAMPLE_INTERVAL * 10.0f);
  msgBuffer[8] = (scaledInterval & 0xFF);
  msgBuffer[9] = (scaledInterval >> 8) & 0xFF;
  uint16_t memidx = 10;
  for (uint16_t pointIndex = 0; pointIndex < SAMPLE_COUNT; pointIndex++)
  {
    uint8_t b[4] = {0};
    memcpy(&msgBuffer[memidx], &sampleBuffer[pointIndex][0], 4);
    memidx += 4;
    memcpy(&msgBuffer[memidx], &sampleBuffer[pointIndex][1], 4);
    memidx += 4;
  }

  mqttPublish("sensor/geo/" + sysConfig.DeviceId, msgBuffer, memidx, QOS0);
  console.println("send");
}

void loop()
{
  /* Common loop will connect if necessary */
  commonLoop();
  assembleGeoPointArray();
  delay(2000);
}