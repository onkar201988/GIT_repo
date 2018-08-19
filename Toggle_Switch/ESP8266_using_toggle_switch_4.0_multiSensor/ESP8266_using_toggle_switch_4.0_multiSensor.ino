#include <ESP8266WiFi.h>
#include <PubSubClient.h>0                                                                              
#include <Servo.h>
#include <CapacitiveSensor.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoOTA.h>
extern "C" {
  #include "user_interface.h"
}

//-------------- String declairation--------------------------
const char* ssid              = "LakeViewWiFi";
const char* password          = "P@ssLakeView";
const char* mqtt_server       = "192.168.2.12";
const char* mqtt_uname        = "onkar20";
const char* mqtt_pass         = "onkar20";
const char* mqtt_device_name  = "ESP8266KitchenSwitch";
const char* ota_device_name   = "OTA_Kitchen_Switch";
const char* ota_password      = "onkar20";

//-------------variable declaration
const int servo     = 4;
const int lightPin  = 2;
const int posOn     = 150;
const int posOff    = 45;
const int posNormal = 100;
int switchStatus    = 0;
int lightIntensity  = 0;
int counter         = 0;

const int DHTPIN    = 12;
const int LDRPIN    = A0;
const int RFPIN     = 5;
const int capSend   = 16;
const int capReceive= 14;
bool  flag          = false;
bool motionFlag     = false;
#define DHTTYPE     DHT11     // DHT 11

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
Servo s1; //servo 1
//CapacitiveSensor capSensor = CapacitiveSensor(capSend, capReceive);

//----------------------------------------------------------------------------------
void setup() {
  s1.attach(servo);
  s1.write(posOff);
  s1.detach();
  pinMode(lightPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(LDRPIN, INPUT);
  pinMode(RFPIN, INPUT);
  pinMode(DHTPIN, INPUT);
  
  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived new [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    blinkLED(2);
    switchStatus = 1;              //update local switch status with MQTT
    runServo(posOn);
    client.publish("home/kitchenSwitch/state", "1");
    //sleep_timer = 4;
  }
  else {
    blinkLED(1);
    switchStatus = 0;              //update local switch status with MQTT
    runServo(posOff);
    client.publish("home/kitchenSwitch/state", "0");
    //sleep_timer = 2;
  }
}
//---------------------------------------------------------------------------------------------------
void runServo(int servoPos) {
  s1.attach(servo);
  s1.write(servoPos);
  delay(500);
  s1.write(posNormal);
  delay(500);
  s1.detach();
}
//---------------------------------------------------------------------------------------------------
void blinkLED (int noOfTimes) {
  for(int i=0; i< noOfTimes; i++) {
    digitalWrite(lightPin, LOW);
    delay(100);
    digitalWrite(lightPin, HIGH);
    delay(100);
  }
}
//----------------------------------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_device_name, mqtt_uname, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe("home/kitchenSwitch/command");
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
void readTemperature()
{
  float newTempValue = dht.readTemperature(true); //to use celsius remove the true text inside the parentheses  
  float newHumValue = dht.readHumidity();

  Serial.print("Temperature: ");
    Serial.print(newTempValue);
    Serial.println(" *F");
    client.publish("home/kitchen/temperature", String(newTempValue).c_str());

  Serial.print("Humidity: ");
    Serial.print(newHumValue);
    Serial.println("%");
    client.publish("home/kitchen/humidity", String(newHumValue).c_str());
}

//----------------------------------------------------------------------------------------------------
void readLight(){
  int newLDR = analogRead(LDRPIN);

  lightIntensity = lightIntensity + 0.25*(newLDR - lightIntensity);
  
  Serial.print("light intensity: ");
  Serial.println(lightIntensity);
  client.publish("home/kitchen/light", String(lightIntensity).c_str());
}

//----------------------------------------------------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  
  if (!client.connected()) {
    reconnect();
  }
  
  //scheduler
  counter++;
  if(counter%20 == 0)
  {
    readTemperature();
  }

  if(counter%15 == 0)
  {
    readLight();
  }

  if(counter > 99)
  {
    counter = 0;
  }

  if(digitalRead(RFPIN))
  {
    if(motionFlag == false)
    {
      Serial.println("motion detected");
      motionFlag = true;
    }
  }
  else
  {
    if(motionFlag == true)
    {
      Serial.println("no motion");
      motionFlag = false;
    }
  }
  //long total = capSensor.capacitiveSensor(30);
  //Serial.print(total);
  
  delay(1000);
  client.loop();
}
