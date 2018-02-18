// DHT Temperature & Humidity Sensor

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoOTA.h>
extern "C" {
  #include "user_interface.h"
}

//-------------- String declairation--------------------------
const char* ssid = "LakeViewWiFi";
const char* password = "P@ssLakeView";
const char* mqtt_server = "192.168.2.12";
const char* mqtt_uname = "onkar20";
const char* mqtt_pass = "onkar20";
const char* mqtt_device_name = "SensorModuleBedroom";
const char* ota_device_name = "OTA_Sensor_module_bedroom";
const char* ota_password = "onkar20";

//-------------variable declaration--------------------------------
WiFiClient espClient;
PubSubClient client(espClient);
#define DHTPIN    4
#define LDRPIN    A0
#define PIRPIN    5
#define DHTTYPE           DHT11     // DHT 11 
const int lightPin = 2;
DHT dht(DHTPIN, DHTTYPE);
int counter = 0;
int lightIntensity = 0;
int filterCycles = 5;
int sleepTime = 1; // in minuts
//-----------------------------------------------------------------
void setup() {
  pinMode(LDRPIN, INPUT);
  pinMode(PIRPIN, INPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(lightPin, OUTPUT); 

  lightIntensity = analogRead(LDRPIN);
  
  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  
  client.setServer(mqtt_server, 1883);
  digitalWrite(lightPin, LOW);
}

//----------------------------------------------------------------------------------------------------
void setup_OTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(ota_device_name);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)ota_password);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

//----------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_device_name, mqtt_uname, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("dev/out", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//----------------------------------------------------------------------------------------------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //for Light sleep
  WiFi.mode(WIFI_STA);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(lightPin, !digitalRead(lightPin));
  }

  digitalWrite(lightPin, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void readTemperature()
{
  float newTempValue = dht.readTemperature(true); //to use celsius remove the true text inside the parentheses  
  float newHumValue = dht.readHumidity();

  if(filterCycles == 1)
  {
    Serial.print("Temperature: ");
      Serial.print(newTempValue);
      Serial.println(" *F");
      client.publish("home/bedroom/temperature", String(newTempValue).c_str());
  
    Serial.print("Humidity: ");
      Serial.print(newHumValue);
      Serial.println("%");
      client.publish("home/bedroom/humidity", String(newHumValue).c_str());
  }
}

void readLight(){
  int newLDR = analogRead(LDRPIN);
  lightIntensity = lightIntensity + 0.25*(newLDR - lightIntensity);
 
  if(filterCycles == 1)
  {
    Serial.print("light intensity: ");                          
    Serial.println(lightIntensity);
    client.publish("home/bedroom/light", String(lightIntensity).c_str());
  }
}

//------------------------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  
  if (!client.connected()) {
    reconnect();
  }
  
  readTemperature();
  readLight();
  filterCycles--;
  
  delay(1000);

  if(filterCycles == 0)
  {
    Serial.println("Going to sleep for 1 Min");
    ESP.deepSleep(sleepTime*60*1000000, WAKE_RF_DEFAULT);
  }
}
