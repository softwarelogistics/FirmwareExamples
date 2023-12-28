#define CAN_ENABLED
#define CAN_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>
#include "esp_err.h"
#include "EngineSensors.h"
#include "driver/gpio.h"
#include "driver/twai.h"

#define EXAMPLE_SKU "CAN_BUS_EXAMPLE"
#define FW_SKU "CAN_BRD_V1"
#define FIRMWARE_VERSION "1.0.0"

uint8_t count = 0;

CANEngineSensors sensor(&console);

void consoleHandler(String cmd)
{
  console.println("Command received: " + cmd);
  delay(100);
  twai_message_t message;
  message.identifier = 5;
  message.data_length_code = 8;
  for (int i = 0; i < 8; i++)
  {
    count++;
    message.data[i] = count;
    message.flags = TWAI_MSG_FLAG_NONE;
  }

  ESP_ERROR_CHECK_WITHOUT_ABORT(twai_transmit(&message, pdMS_TO_TICKS(1000)));
}

void setup()
{
  initPins();

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)configPins.CANTx, (gpio_num_t)configPins.CANRx, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  configureConsole();
  configureFileSystem();

  ioConfig.load();
  sysConfig.load();
  sysConfig.SendUpdateRateMS = 5000;

  console.registerCallback(consoleHandler);

  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "CAN Bus BT Example" : sysConfig.DeviceId);
  BT.begin(btName.c_str(), FW_SKU);

  ESP_ERROR_CHECK_WITHOUT_ABORT(twai_driver_install(&g_config, &t_config, &f_config));

  // Start CAN driver
  if (twai_start() == ESP_OK)
  {
    console.println("Driver started\n");
  }
  else
  {
    console.println("Failed to start driver\n");
    return;
  }

  uint32_t alerts_to_enable = TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_OFF;
  ESP_ERROR_CHECK_WITHOUT_ABORT(twai_reconfigure_alerts(alerts_to_enable, NULL));

  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cns001", 010);
}

int nextPrint = 0;
int idx = 0;

uint32_t __nextCanSend = 0;

uint16_t fuelRate = 1523;
uint16_t fuelPressure = 1423;
uint16_t manifoldPressure = 29.92;
bool bilgeOn = false;
bool highWaterDetection = false;
float coolantFlow = 80.4;

void sendBatteryLevels(int batteryInstance, float voltage)
{
  twai_message_t message;
  message.identifier = 0x9F21440;
  message.data_length_code = 8;
  message.extd = 1;

  message.data[0] = batteryInstance;
  int intVoltage = voltage * 100;
  message.data[1] = intVoltage & 0xFF;
  message.data[2] = (intVoltage >> 8) & 0xFF;
  message.data[3] = 0xFF;
  message.data[4] = 0xFF;
  message.data[5] = 0xFF;
  message.data[6] = 0xFF;
  message.data[7] = 0xFF;

  ESP_ERROR_CHECK_WITHOUT_ABORT(twai_transmit(&message, pdMS_TO_TICKS(1000)));
}

void sendCustomEngineProperties()
{
    twai_message_t message;
    message.identifier = 0x9E20140;
    message.data_length_code = 8;
    message.extd = 1;

    for (uint8_t j = 1; j < 8; ++j)
      message.data[j] = 0xFF;

    message.data[0] = 0;
    message.data[1] = 1;
    message.data[2] = fuelRate & 0xFF;
    message.data[3] = (fuelRate >> 8) & 0xFF;
    message.data[4] = fuelPressure & 0xFF;
    message.data[5] = (fuelPressure >> 8) & 0xFF;
    message.data[6] = manifoldPressure & 0xFF;
    message.data[7] = (manifoldPressure >> 8) & 0xFF;
    ESP_ERROR_CHECK_WITHOUT_ABORT(twai_transmit(&message, pdMS_TO_TICKS(1000)));
}

void sendCustom2Properties() {
    twai_message_t message;
    message.identifier = 0x9E20240;
    message.data_length_code = 8;
    message.extd = 1;

    for (uint8_t j = 1; j < 8; ++j)
      message.data[j] = 0xFF;

    message.data[0] = 0;
    message.data[1] = 1;
    message.data[2] = 0x00;
    if(bilgeOn)
      message.data[2] |= 0b00000010;

    if(highWaterDetection)
      message.data[2] |= 0b00000010;

    message.data[3] = (int)(coolantFlow * 100) & 0xFF;
    message.data[4] = ((int)(coolantFlow * 100) >> 8) & 0xFF;
    ESP_ERROR_CHECK_WITHOUT_ABORT(twai_transmit(&message, pdMS_TO_TICKS(1000)));
}

void loop()
{
  commonLoop();

  if (millis() > __nextCanSend && sysConfig.DeviceId != "candev01")
  {
    __nextCanSend = millis() + 500;

    sendCustomEngineProperties();
    delay(50);
    sendBatteryLevels(1, 12 + ((float)random(-10, 10) / 10.0));
    delay(50);
    sendBatteryLevels(2, 12 + ((float)random(-10, 10) / 10.0));
    delay(50);
    sendBatteryLevels(3, 12 + ((float)random(-10, 10) / 10.0));
    delay(50);
    sendBatteryLevels(4, 12 + ((float)random(-10, 10) / 10.0));    
    delay(50);
    sendBatteryLevels(5, 12 + ((float)random(-10, 10) / 10.0));
    delay(50);
    sendCustom2Properties();
  }

  twai_message_t message;
  if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
  {
    if (!(message.rtr))
    {
      BT.writeCANMessage(message.identifier, message.data, message.data_length_code);
      console.setVerboseLogging(false);
    }
  }

  /*

    uint32_t alerts_triggered;
    if(twai_read_alerts(&alerts_triggered, portMAX_DELAY) == ESP_OK){
      if(alerts_triggered & TWAI_ALERT_ERR_PASS){
      if(alerts_triggered & TWAI_ALERT_ERR_PASS){
        console.println("TWAI_ALERT_ERR_PASS");
      }
      else if(alerts_triggered & TWAI_ALERT_BUS_OFF){
        console.println("TWAI_ALERT_BUS_OFF");
      }
      else if(alerts_triggered && TWAI_ALERT_TX_IDLE) {
        console.println(String(alerts_triggered));
      }
    }
    }*/
}