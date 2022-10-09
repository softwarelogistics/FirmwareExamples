#include <Arduino.h>
#include "PubSubClient.h"
#include <WiFiClient.h>
#include <uri/UriRegex.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>
#include "valve.h"
#include "pages.h"

const char *host = "pool-valves";
const char *ssid = "CasaDeWolf";
const char *password = "TheWolfBytes";

const char *VERSION = "2.1.0";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

IPAddress _mqttServer;
WiFiClient _wifiClient;
PubSubClient _mqtt(_wifiClient, _mqttServer);

Valve jets(2, 14, "spa", "both", "normal");
Valve source(16, 5, "pool", "both", "spa");
Valve output(4, 0, "pool", "both", "spa");

const int MOTOR_TEMP = A0;

unsigned long next_send = 0;

void connectWiFi()
{
  WiFi.begin("SLManCave", "TheWolfBytes");
  ssid = "SLManCave";

  int attempt = 0;

  Serial.println();
  Serial.println();
  Serial.print("Connecting to " + String(ssid));

  int status = WiFi.status();

  while (status != WL_CONNECTED){
    attempt++;
    if (attempt == 20){
      WiFi.begin("ByteMaster", "TheWolfBytes");
      ssid = "ByteMaster";
    }
    else if (attempt == 40)
    {
      WiFi.begin("SoftwareLogistics", "TheWolfBytes");
      ssid = "SoftwareLogistics";
    }
    else if (attempt == 60)
    {
      WiFi.begin("CasaDeWolf", "TheWolfBytes");
      ssid = "CasaDeWolf";
      attempt = 0;
    }

    delay(500);
    Serial.print(".");
    status = WiFi.status();
  }

  Serial.println();

  Serial.print("Connected To: ");
  Serial.println(ssid);
  Serial.print("IP Address  : ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address : ");
  Serial.println(WiFi.macAddress());
  Serial.print("DNS Address : ");
  Serial.println(WiFi.dnsIP());
  Serial.print("Subnet Mask : ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gate Way    : ");
  Serial.println(WiFi.gatewayIP());

  EEPROM.begin(12);
  uint16_t initValue;

  EEPROM.get(0, initValue);
  if(initValue != 0x4321){
    int defaultValue = 14000;
    Serial.println("EEPROM Not set, initialize values");
    EEPROM.put(0, (uint16_t)0x4321);
    EEPROM.put(sizeof(uint16_t), defaultValue);
    EEPROM.put(sizeof(uint16_t) * 2, defaultValue);
    EEPROM.put(sizeof(uint16_t) * 4, defaultValue);
  }
  else {
    Serial.println("EEPROM has valid content");
  }

  uint16_t timing = 0;

  EEPROM.get(sizeof(uint16_t), timing);  
  source.setTiming(timing);

  EEPROM.get(sizeof(uint16_t) * 2, timing);
  output.setTiming(timing);

  EEPROM.get(sizeof(uint16_t) * 4, timing);
  jets.setTiming(timing);

  EEPROM.end();
}

String _deviceId = "pool001";
String _topicPrefix = "pool001/valves/";

void handleTopic(String cmd){
  Serial.println("Handling topic  " + String(cmd));

  if (cmd == "source/pool") source.setCurrentPosition(currentPosition_0);
  else if (cmd == "source/both") source.setCurrentPosition(currentPosition_90);
  else if (cmd == "source/spa") source.setCurrentPosition(currentPosition_180);
  else if (cmd == "output/pool") output.setCurrentPosition(currentPosition_0);  
  else if (cmd == "output/spa") output.setCurrentPosition(currentPosition_180);
  else if (cmd == "output/both") output.setCurrentPosition(currentPosition_90);
  else if (cmd == "spa/normal") jets.setCurrentPosition(currentPosition_180);
  else if (cmd == "spa/both") jets.setCurrentPosition(currentPosition_90);
  else if (cmd == "spa/jets") jets.setCurrentPosition(currentPosition_0);
}

void callback(const MQTT::Publish &pub){
  if (_topicPrefix.length() > 0){
    String cmd = pub.topic().substring(_topicPrefix.length());
    handleTopic(cmd);
  }
}

void connectMQTT(){
  _mqtt.disconnect();
  _mqtt.set_server("mqttdev.nuviot.com");

  if (_mqtt.connect(MQTT::Connect(_deviceId + "/valves").set_auth("kevinw", "Test1234"))){
    Serial.println("Connected to MQTT server");
    Serial.print("Subscribing to topic: ");
    Serial.println(_topicPrefix + "#");

    _mqtt.set_callback(callback);

    if (_mqtt.subscribe(_topicPrefix + "#"))
      Serial.println("Subscribed!");
    else
      Serial.println("Did not subscribe");
  }
  else
    Serial.println("Could not connect to MQTT server.");
}

void redirectToSensorPage() {
  httpServer.sendHeader("Location", String("/sensor"), true);
  httpServer.send(302, "text/plain");
}

void setTiming(String port, String timing) {
  EEPROM.begin(12);
  uint16_t newTiming = atoi(timing.c_str()) * 1000;

  if(port == "source") {
    source.setTiming(newTiming);
    EEPROM.put(sizeof(uint16_t), newTiming);
  }
  else if(port == "output") {
    output.setTiming(newTiming);
    EEPROM.put(sizeof(uint16_t) * 2, newTiming);
  }
  else if(port == "jets") {
    jets.setTiming(newTiming);
    EEPROM.put(sizeof(uint16_t) * 4, newTiming);
  }
  EEPROM.end();
}

void handleRoot(){
  httpServer.send(200, "text/plain", "hello from esp8266!");
}

void setupWebServer(){
  httpServer.on("/", HTTP_GET, []() { httpServer.sendHeader("Connection", "close"); httpServer.send(200, "text/html", loginIndex); });
  httpServer.on("/serverIndex", HTTP_GET, [](){ httpServer.sendHeader("Connection", "close"); httpServer.send(200, "text/html", serverIndex); });
  httpServer.on("/status", HTTP_GET, []() { sendSensorData(); });
  httpServer.on("/sensor", sendSensorData);
  httpServer.on(UriRegex("\/set\/([a-z]+)\/([a-z]+)"), HTTP_GET, [&]() { handleTopic(httpServer.pathArg(0) + "/" + httpServer.pathArg(1)); redirectToSensorPage();} );
  httpServer.on(UriRegex("\/settimeout\/([a-z]+)\/([0-9]+)"), HTTP_GET, [&]() { setTiming(httpServer.pathArg(0), httpServer.pathArg(1));  redirectToSensorPage();} );
  httpUpdater.setup(&httpServer);
  httpServer.begin();
}

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");

  connectWiFi();
  connectMQTT();

  setupWebServer();

  if (!MDNS.begin(host)){ // http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1){
      delay(1000);
    }
  }

  Serial.println("mDNS responder started");

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready!\r\nOpen http://%s.local/update in your browser\r\n", host);

  next_send = millis() + 50000;
}

void loop(void)
{
  int motorTempCount = analogRead(A0);
  delay(25);

  jets.Update();
  source.Update();
  output.Update();

  int status = WiFi.status();

  if (status != WL_CONNECTED){
    Serial.print("WiFi Connection Lost: ");
    switch (status)
    {
      case WL_NO_SHIELD: Serial.println("No Shield."); break;
      case WL_IDLE_STATUS: Serial.println("Idle Status."); break;
      case WL_NO_SSID_AVAIL: Serial.println("No SSID Available."); break;
      case WL_SCAN_COMPLETED: Serial.println("Scan Completed."); break;
      case WL_CONNECT_FAILED: Serial.println("Connect Failed."); break;
      case WL_CONNECTION_LOST: Serial.println("Connection Lost."); break;
      case WL_DISCONNECTED: Serial.println("Disconnected.");break;
    }
    connectWiFi();
  }
  else if (!_mqtt.connected()){
    Serial.println("MQTT Connection Lost.");
    connectMQTT();
  }
  else{
    if (next_send < millis()){
      String topic = "pool/valvestat/" + String(_deviceId);
      String msg = "{\"deviceId\":\"" +
                   String(_deviceId) +
                   "\",\"ipaddress\":\"" +
                   WiFi.localIP().toString() +
                   "\",\"source\":\"" +
                   source.getStatus() +
                   "\",\"output\":\"" +
                   output.getStatus() + "\",\"spa\":" + jets.getStatus() + "\", \"motorTemp\":" + String(motorTempCount) + "}";
      _mqtt.publish(topic, msg);
      next_send = millis() + 5000;
    }

    httpServer.handleClient();
    _mqtt.loop();
  }
}
