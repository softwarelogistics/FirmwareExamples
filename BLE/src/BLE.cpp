#include "BLE.h"
#include <NuvIotState.h>

// List of BLE Services:
// https://www.bluetooth.com/specifications/specs/
//

#define SVC_UUID_NUVIOT "d804b639-6ce7-4e80-9f8a-ce0f699085eb"

#define CHAR_UUID_STATE "d804b639-6ce7-5e81-9f8a-ce0f699085eb"
/* 
 * Stete characteristic will encompass 
 * Read/Write and Will Notify
 *
 * xxxx => F/W SKU
 * xxx.xxx.xxx, F/W Version =>
 * xxx.xxx.xxx, H/W Version =>
 
 * (1/0) <= 1 to reboot
 * (1/0) <= => Commmissioned
 * (1/0) => Internet Connectivity
 * (1/0) => Server Connectivity
 * xxx => OTA State
 * xxx => OTA Param
 */

#define CHAR_UUID_SYS_CONFIG "d804b639-6ce7-5e82-9f8a-ce0f699085eb"
/* 
  * Sys Config characteristic
  * Read/Write
  * xxxxx, Device Id <= =>
  * xxxxx, B64 Device Key (128 characters) =>
  * (0/1) Cell Enable <= =>
  * (0/1) WiFi Enable <= =>
  * xxxxxx WiFi SSID <= =>
  * xxxxxx WiFi Password =>
  * xxxx Ping Rate (sec)
  * xxxx Send Rate (sec)
  * (0/1) GPS Enable
  * xxxx GPS Rate (sec),
  */

#define CHAR_UUID_IOCONFIG "d804b639-6ce7-5e83-9f8a-ce0f699085eb"
/* IO Config
   * 
   * 8 Slots
   * 3 Params per slot
   * x = Configuration
   * xxx = scale
   * xxx = zero
   *
   */

#define CHAR_UUID_ADC_IOCONFIG "d804b639-6ce7-5e84-9f8a-ce0f699085eb"
/* ADC Config
   * 
   * 8 Slots
   * 3 Params per slot
   * x = Configuration
   * xxx = scale
   * xxx = zero
   *
   */

#define CHAR_UUID_IO_VALUE "d804b639-6ce7-5e85-9f8a-ce0f699085eb"
/* IO Config
   * 
   * 8 Slots
   * 3 Params per slot
   * x = Configuration
   * xxx = scale
   * xxx = zero
   *
   */

#define CHAR_UUID_ADC_VALUE "d804b639-6ce7-5e86-9f8a-ce0f699085eb"
/* ADC Config
   * 
   * 8 Slots
   * 3 Params per slot
   * x = Configuration
   * xxx = scale
   * xxx = zero
   *
   */

#define CHAR_UUID_RELAY "d804b639-6ce7-5e87-9f87-ce0f699085eb"
/* RELAY Config
   * 
   * 16 slots
   * (1,0) <= => Relay State
   *
   */

#define CHAR_UUID_CONSOLE "d804b639-6ce7-5e88-9f88-ce0f699085eb"
/* RELAY Config
   * 
   * 16 slots
   * (1,0) <= => Relay State
   *
   */

#define FULL_PACKET 512
#define CHARPOS_UPDATE_FLAG 5

esp_ota_handle_t otaHandler = 0;

bool updateFlag = false;
bool readyFlag = false;
int bytesReceived = 0;
int timesWritten = 0;

void otaCallback::onRead(BLECharacteristic *pCharacteristic)
{
  pBle->handleReadCharacteristic(pCharacteristic);
}

void otaCallback::onNotify(BLECharacteristic *pCharacteristic)
{
  pBle->handleNotifyCharacteristic(pCharacteristic);
}

void otaCallback::onWrite(BLECharacteristic *pCharacteristic)
{
  pBle->handleWriteCharacteristic(pCharacteristic);
}

void BLECustomServerCallbacks::onConnect(BLEServer *pServer)
{
  delay(1000);

  pBle->clientConnected();
};

void BLE::refreshCharacteristics()
{
  String state =
      pState->getFirmwareSKU() + "," +
              pState->getFirmwareVersion() + "," +
              pState->getHardwareRevision() + "," +
              "0," + /* Reboot - send zero */
              (pSysConfig->Commissioned
          ? "1,"
      : "0,") +
              (pState->getIsWANConnected()
          ? "1,"
      : "0,") +
              (pState->getIsCloudConnected()
          ? "1,"
          : "0,") +
                String(pState->OTAState) + "," +
                String(pState->OTAParam);

  pConsole->println(state);

  pCharState->setValue(state.c_str());

  String config =
      pSysConfig->DeviceId + "," +
              pSysConfig->DeviceAccessKey + "," +
              (pSysConfig->CellEnabled
          ? "1,"
      : "0,") +
              (pSysConfig->WiFiEnabled
          ? "1,"
      : "0,") +
              pSysConfig->WiFiSSID + "," +
              pSysConfig->WiFiPWD + "," +
              String(pSysConfig->PingRate) + "," +
              String(pSysConfig->SendUpdateRate) + "," +
              (pSysConfig->GPSEnabled
          ? "1,"
          : "0,") +
                String(pSysConfig->GPSUpdateRate);

  pConsole->println(config);

  pCharConfig->setValue(config.c_str());

  String ioConfig =
      String(pIOConfig->GPIO1Config) + "," +
      String(pIOConfig->GPIO1Zero) + "," +
      String(pIOConfig->GPIO1Scaler) + "," +
      String(pIOConfig->GPIO2Config) + "," +
      String(pIOConfig->GPIO2Zero) + "," +
      String(pIOConfig->GPIO2Scaler) + "," +
      String(pIOConfig->GPIO3Config) + "," +
      String(pIOConfig->GPIO3Zero) + "," +
      String(pIOConfig->GPIO3Scaler) + "," +
      String(pIOConfig->GPIO4Config) + "," +
      String(pIOConfig->GPIO4Zero) + "," +
      String(pIOConfig->GPIO4Scaler) + "," +
      String(pIOConfig->GPIO5Config) + "," +
      String(pIOConfig->GPIO5Zero) + "," +
      String(pIOConfig->GPIO5Scaler) + "," +
      String(pIOConfig->GPIO6Config) + "," +
      String(pIOConfig->GPIO6Zero) + "," +
      String(pIOConfig->GPIO6Scaler) + "," +
      String(pIOConfig->GPIO7Config) + "," +
      String(pIOConfig->GPIO7Zero) + "," +
      String(pIOConfig->GPIO7Scaler) + "," +
      String(pIOConfig->GPIO8Config) + "," +
      String(pIOConfig->GPIO8Zero) + "," +
      String(pIOConfig->GPIO8Scaler);

  pConsole->println(ioConfig);

  pCharIOConfig->setValue(config.c_str());

  String adcConfig =
      String(pIOConfig->ADC1Config) + "," +
      String(pIOConfig->ADC1Zero) + "," +
      String(pIOConfig->ADC1Scaler) + "," +
      String(pIOConfig->ADC2Config) + "," +
      String(pIOConfig->ADC2Zero) + "," +
      String(pIOConfig->ADC2Scaler) + "," +
      String(pIOConfig->ADC3Config) + "," +
      String(pIOConfig->ADC3Zero) + "," +
      String(pIOConfig->ADC3Scaler) + "," +
      String(pIOConfig->ADC4Config) + "," +
      String(pIOConfig->ADC4Zero) + "," +
      String(pIOConfig->ADC4Scaler) + "," +
      String(pIOConfig->ADC5Config) + "," +
      String(pIOConfig->ADC5Zero) + "," +
      String(pIOConfig->ADC5Scaler) + "," +
      String(pIOConfig->ADC6Config) + "," +
      String(pIOConfig->ADC6Zero) + "," +
      String(pIOConfig->ADC6Scaler) + "," +
      String(pIOConfig->ADC7Config) + "," +
      String(pIOConfig->ADC7Zero) + "," +
      String(pIOConfig->ADC7Scaler) + "," +
      String(pIOConfig->ADC8Config) + "," +
      String(pIOConfig->ADC8Zero) + "," +
      String(pIOConfig->ADC8Scaler);

  pConsole->println(adcConfig);

  pCharADCConfig->setValue(config.c_str());

  String relay;
  (pRelayManager->getRelayState(0) ? "1," : "0,") + 
  (pRelayManager->getRelayState(1) ? "1," : "0,") + 
  (pRelayManager->getRelayState(2)   ? "1,": "0,") + 
  (pRelayManager->getRelayState(3)   ? "1," : "0,") + 
  (pRelayManager->getRelayState(4)   ? "1" : "0");

  pCharRelay->setValue(relay.c_str());
}

void BLECustomServerCallbacks::onDisconnect(BLEServer *pServer)
{
  BLEDevice::startAdvertising();
  pConsole->println("Disconnected");
  pBle->clientDisconnected();
}

void BLE::handleReadCharacteristic(BLECharacteristic *characteristic)
{
  refreshCharacteristics();
}

void BLE::handleWriteCharacteristic(BLECharacteristic *characteristic)
{
  const char *uuid = characteristic->getUUID().toString().c_str();

  if (0 == strcmp(uuid, CHAR_UUID_STATE))
  {
    // pSysConfig->Commissioned = characteristic->getData()[0] == '1';
  }

  if (0 == strcmp(uuid, CHAR_UUID_SYS_CONFIG))
  {
    String input = String(characteristic->getValue().c_str());
    pConsole->println(input);
  }

  //pIOConfig->write();
  //pSysConfig->write();

  //pConsole->println(String(characteristic->getUUID().toString().c_str()));
  //pConsole->println(String((char *)characteristic->getData()));*/
}

void BLE::handleNotifyCharacteristic(BLECharacteristic *characteristic)
{
  /*if (characteristic->getUUID().toString() == CHAR_UUID_PROVISION_DEVICE_ID)
  {
    pConsole->println("handle notify device id");
  }
  else
  {
    pConsole->println("handle notify OTHER");
  }*/
}

//
// begin
bool BLE::begin(const char *localName)
{
  pConsole->println("PRE ALLOC " + String(ESP.getFreeHeap()));

  // Create the BLE Device
  BLEDevice::init(localName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLECustomServerCallbacks(this, pConsole));

  pService = pServer->createService(BLEUUID(SVC_UUID_NUVIOT));

  pCharState = pService->createCharacteristic(CHAR_UUID_STATE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharState->setCallbacks(_characteristicCallback);

  pCharConfig = pService->createCharacteristic(CHAR_UUID_SYS_CONFIG, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharConfig->setCallbacks(_characteristicCallback);

  pCharIOConfig = pService->createCharacteristic(CHAR_UUID_IOCONFIG, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharIOConfig->setCallbacks(_characteristicCallback);

  pCharADCConfig = pService->createCharacteristic(CHAR_UUID_ADC_IOCONFIG, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharADCConfig->setCallbacks(_characteristicCallback);

  pCharIOValue = pService->createCharacteristic(CHAR_UUID_IO_VALUE, BLECharacteristic::PROPERTY_NOTIFY);
  pCharIOValue->setCallbacks(_characteristicCallback);

  pCharADCValue = pService->createCharacteristic(CHAR_UUID_ADC_VALUE, BLECharacteristic::PROPERTY_NOTIFY);
  pCharADCValue->setCallbacks(_characteristicCallback);

  pCharRelay = pService->createCharacteristic(CHAR_UUID_RELAY, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharRelay->setCallbacks(_characteristicCallback);

  pCharConsole = pService->createCharacteristic(CHAR_UUID_CONSOLE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharConsole->setCallbacks(_characteristicCallback);

  pConsole->println("AFTER ALLOC " + String(ESP.getFreeHeap()));

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SVC_UUID_NUVIOT);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  pService->start();
  pAdvertising->start();

  pConsole->println("AFTER STARTUP " + String(ESP.getFreeHeap()));

  return true;
}

void BLE::update()
{
  idx++;

  if (m_isConnected)
  {
    pConsole->println(F("Client Connected"));
    /*    uint8_t adcValues[8] = {(uint8_t)idx, HARDWARE_VERSION_MINOR, SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH, 5, 6, 7};
    pSensorADCCharecteristics->setValue(adcValues, 8);
    pSensorADCCharecteristics->notify(true);
    pRelaysCharacteristics->setValue(adcValues, 8);
    pRelaysCharacteristics->notify(true);
    pSensorGPIIOCharecteristics->setValue(adcValues, 8);
    pSensorGPIIOCharecteristics->notify(true);*/
  }
  else
  {
    pConsole->println(F("Client Not Connected"));
  }
}

//
// Destructor
BLE::~BLE(void)
{
}
