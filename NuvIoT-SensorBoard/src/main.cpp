#define DHT_DEBUG

#define TEMP_SNSR_BOARD_V3

#include <TemperatureProbes.h>
#include <Arduino.h>
#include <NuvIoT.h>

#define TEMP_SNSR_SKU "RSB-01"
#define FIRMWARE_VERSION "0.9.0"
#define HARDWARE_REVISION "4.0"

#define BATT_SENSE_PIN 34
#define TEMP_SENSE_PIN 35

#define IO1_PIN 25
#define IO2_PIN 26

bool hasDHT22 = false;
bool hasProbe1 = false;
bool hasProbe2 = false;

NuvIoT_DHT *dht;
DallasTemperature *probe1;
DallasTemperature *probe2;

byte buffer[8];
bool running = true;

void determineSensorConfiguration()
{ 
  dht = new NuvIoT_DHT(IO1_PIN, DHT22, 6, &console);
  dht->begin();
  uint8_t retryCount = 0;
  while (retryCount++ < 5 && !hasDHT22)
  {
    float temp = dht->readTemperature(true,true);
    if (!isnan(temp)){
      hasDHT22 = true;
      console.println("Found DHT22");
    }
    else {
      console.println("Attempt " + String(retryCount));
      delay(1000);
    }
  }

  if (!hasDHT22){
    console.println("Did Not Find DHT22");  
    delete dht;
    dht = NULL;
  }
  else{
    ioConfig.GPIO1Config = GPIO_CONFIG_DHT22;
    ioConfig.GPIO1Name = "Digital Temperature";
    ioConfig.GPIO1Scaler = 1;
    ioConfig.GPIO1Zero = 0;
    ioConfig.GPIO1Calibration = 1;

    ioConfig.GPIO2Config = GPIO_CONFIG_DHT22_HUMIDITY;
    ioConfig.GPIO2Name = "Digital Humidity";
    ioConfig.GPIO2Scaler = 1;
    ioConfig.GPIO2Zero = 0;
    ioConfig.GPIO2Calibration = 1;
  }

  if (dht == NULL){
    probe1 = new DallasTemperature(new OneWire(IO1_PIN));
    retryCount = 0;
    while (retryCount++ < 5 && !hasProbe1)
    {
      float temp = probe1->getTempFByIndex(0);
      if (!isnan(temp) && temp > -50.60f)
      {
        console.println("actual probe 2 response " + String(temp));
        hasProbe1 = true;
      }
    }

    if (!hasProbe1){
      probe1 = NULL;
      console.println("Does not have DS18B Probe 1");
    }
    else{
      console.println("Has DS18B Probe 1");
      ioConfig.GPIO1Config = GPIO_CONFIG_DBS18;
      if (ioConfig.GPIO1Name == "")
        ioConfig.GPIO1Name = "Digital Temperature - Port 1";
      ioConfig.GPIO1Scaler = 1;
      ioConfig.GPIO1Zero = 0;
      ioConfig.GPIO1Calibration = 1;
    }

    probe2 = new DallasTemperature(new OneWire(IO2_PIN));
    retryCount = 0;
    while (retryCount++ < 5 && !hasProbe2)
    {
      float temp = probe2->getTempFByIndex(0);
      if (!isnan(temp) && temp > -50.60f)
      {
        console.println("actual probe 2 response " + String(temp));
        hasProbe2 = true;
      }
    }

    if (!hasProbe2)
    {
      probe2 = NULL;
      console.println("Does not have DS18B Probe 2");
    }
    else
    {
      console.println("Has DS18B Probe 2");
      ioConfig.GPIO2Config = GPIO_CONFIG_DBS18;
      if (ioConfig.GPIO2Name == "")
        ioConfig.GPIO2Name = "Digital Temperature - Port 2";
      ioConfig.GPIO2Scaler = 1;
      ioConfig.GPIO2Zero = 0;
      ioConfig.GPIO2Calibration = 1;
    }

    ioConfig.ADC1Config = ADC_CONFIG_ADC;
    ioConfig.ADC1Name = "Analog Temperature";
    ioConfig.ADC1Scaler = 100;
    ioConfig.ADC1Zero = 32;
    ioConfig.ADC1Calibration = 1;    
  }
}

void setup()
{
  delay(1000);

  configureFileSystem();

  ioConfig.load();
  sysConfig.load();
  sysConfig.WiFiEnabled = true;
  sysConfig.SendUpdateRate = 1000;
  state.init(TEMP_SNSR_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);
  
  initPins();

  configureConsole();
  writeConfigPins();
  determineSensorConfiguration();

  console.registerCallback(handleConsoleCommand);
  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Temp Sensor" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), TEMP_SNSR_SKU);

  wifiMgr.setup();  

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);  
  ledManager.setErrFlashRate(0);

  probes.configure(&ioConfig);
}

int nextPrint = 0;
int idx = 0;
long nextSend = 0;

void loop(){
  uint32_t adcCountsBatt = analogRead(BATT_SENSE_PIN);
  uint32_t adcCountsTemperature = analogRead(TEMP_SENSE_PIN);
  float vin = ((adcCountsBatt / 4096.0f) * 3.3f) * 2.0f;
  float vtemp = ((adcCountsTemperature / 4096.0f) * 3.3f);

  state.setInputVoltage(vin);

  console.println("ADC " + String(vin) + ", " + String(vtemp));

  console.loop();
  ledManager.loop();
  wifiMgr.loop();
  hal.loop();
  BT.update();
  probes.loop();

  if(sysConfig.Commissioned && nextSend < millis())
  {
    String url = "http://" + sysConfig.SrvrHostName + ":" + sysConfig.Port + "/temperature/" + sysConfig.DeviceId;
    console.println(url);
    String body = "{}";
  
    if(hasDHT22){
       body = "{'temperature':" + String(probes.getTemperature(0)) + ",'humidity':" + probes.getHumidity(0) + "}";
    }

    if(hasProbe1 && !hasProbe2) {
      body = "{'temperature':" + String(probes.getTemperature(0)) + "}";
    }
    
    if(hasProbe2 && !hasProbe1) {
      body = "{'temperature':" + String(probes.getTemperature(1)) + "}";
    }

    if(hasProbe2 && hasProbe1) {
      body = "{'temperature':" + String(probes.getTemperature(1)) + ",'temperature2':" + String(probes.getTemperature(1)) + "}";
    }

    wifiMgr.post(sysConfig.SrvrHostName,sysConfig.Port, "/sensor/" + sysConfig.DeviceId + "/temperature", body);
    nextSend = millis() + 1500;
  }

  console.setVerboseLogging(true);
  probes.debugPrint();
  delay(1000);
}