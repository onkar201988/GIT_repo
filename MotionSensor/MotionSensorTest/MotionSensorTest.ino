//MotionSensorTest

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
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
const char* mqtt_device_name = "MotionSensorTest";
const char* ota_device_name = "OTA_MotionSensorTest";
const char* ota_password = "onkar20";

//-------------variable declaration--------------------------------
WiFiClient espClient;
PubSubClient client(espClient);
#define PIRPIN    10
#define LDRPIN    A0
#define RFPIN     5
const int lightPin = 2;
int lightIntensity = 0;
bool rfMotionDetected = false;
bool pirMotionDetected = false;
int counter = 2;

//----------------------------------------------------------------------------------------------------
void setup() {
  pinMode(LDRPIN, INPUT);
  
  pinMode(PIRPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIRPIN), PIRInterruptOn, RISING);
  //attachInterrupt(digitalPinToInterrupt(PIRPIN), PIRInterruptOff, FALLING);
  pinMode(RFPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RFPIN), RFInterruptOn, RISING);
  //attachInterrupt(digitalPinToInterrupt(RFPIN), RFInterruptOff, FALLING);
  
  pinMode(lightPin, OUTPUT);
  
  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  
  client.setServer(mqtt_server, 1883);
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

//----------------------------------------------------------------------------------------------------
void PIRInterruptOn () {
  Serial.println("PIR motion detected");
  counter = 2;
  
//  if(!pirMotionDetected){
//    pirMotionDetected = true;
//    client.publish("home/PIRTest", "1");
//  }
}

//----------------------------------------------------------------------------------------------------
void PIRInterruptOff () {
  Serial.println("PIR motion stopped");
  counter = 2;
  
//  if(!pirMotionDetected){
//    pirMotionDetected = true;
//    client.publish("home/PIRTest", "1");
//  }
}

//----------------------------------------------------------------------------------------------------
void RFInterruptOn () {
  Serial.println("RF motion Detected");
  counter = 2;
  
//  if(!rfMotionDetected){
//    rfMotionDetected = true;
//    client.publish("home/RFTest", "1");
//  }
}

//----------------------------------------------------------------------------------------------------
void RFInterruptOff () {
  Serial.println("RF motion Stopped");
  counter = 2;
  
//  if(!rfMotionDetected){
//    rfMotionDetected = true;
//    client.publish("home/RFTest", "1");
//  }
}
//----------------------------------------------------------------------------------------------------
void readLight(){
  lightIntensity = analogRead(LDRPIN);
  
  Serial.print("light intensity: ");
  Serial.println(lightIntensity);
  client.publish("home/Test/light", String(lightIntensity).c_str());
}
//----------------------------------------------------------------------------------------------------
              
void loop() {
  ArduinoOTA.handle();
  
  if (!client.connected()) {
    reconnect();
  }

  readLight();

  if(digitalRead(RFPIN) == 0)
  {
    Serial.println("RF motion Stopped");
  }
//  if(counter == 0){
//    if(rfMotionDetected){
//      rfMotionDetected = false;
//      client.publish("home/RFTest", "0");
//    }
//    if(pirMotionDetected){
//      pirMotionDetected = false;
//      client.publish("home/PIRTest", "0");
//    }
//  }
  
  counter--;
  delay(2000);
}
