#define CAN_ENABLED
#define CAN_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/twai.h"


#define EXAMPLE_SKU "CAN_BUS_EXAMPLE"
#define FW_SKU "CAN_BRD_V1"
#define FIRMWARE_VERSION "1.0.0"

uint8_t count = 0;

void consoleHandler(String cmd){
  console.println("Command received: " + cmd);
  delay(100);
  twai_message_t  message;
  message.identifier = 5;
  message.data_length_code = 8;
  for (int i = 0; i < 8; i++) {
      count++;
      message.data[i] = count;
      message.flags = TWAI_MSG_FLAG_NONE;
  }

  twai_status_info_t status;

  ESP_ERROR_CHECK_WITHOUT_ABORT(twai_get_status_info(&status));

  console.println("Current status: " + String(status.state));

  esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK_WITHOUT_ABORT(err);
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

void loop()
{  
  commonLoop();

  twai_message_t  message;
  if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
  {
    if (!(message.rtr))
    {
        console.setVerboseLogging(true);
        console.print("Message received id: " + String(message.identifier) + " len: " + String(message.data_length_code) + " data: ");
        console.printByteArray(message.data, message.data_length_code);      

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