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
const char* ssid              = "LakeViewWiFi";
const char* password          = "P@ssLakeView";
const char* mqtt_server       = "192.168.2.12";
const char* mqtt_uname        = "onkar20";
const char* mqtt_pass         = "onkar20";
const char* mqtt_device_name  = "ESP8266ComputerSwitch";
const char* ota_device_name   = "OTA_Computer_Switch";
const char* ota_password      = "onkar20";

//-------------variable declaration
const int relay     = 12;
const int ledPin    = 13;
const int switchPin = 0;
int switchStatus    = 0;
bool firstTime      = true;

WiFiClient espClient;
PubSubClient client(espClient);
//----------------------------------------------------------------------------------
void setup() {
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT);

  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  client.publish("home/computerSwitch/command", "1");
  client.publish("home/computerSwitch/state", "1");
  
  digitalWrite(ledPin, HIGH);
  digitalWrite(switchPin, HIGH);
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
    digitalWrite(ledPin, !digitalRead(ledPin));
  }

  digitalWrite(ledPin, HIGH);
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

  if(!firstTime)
  {
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      blinkLED(2);
      switchStatus = 1;              //update local switch status with MQTT
      digitalWrite(relay, HIGH);
      client.publish("home/computerSwitch/state", "1");
    }
    else {
      blinkLED(1);
      switchStatus = 0;              //update local switch status with MQTT
      digitalWrite(relay, LOW);
      client.publish("home/computerSwitch/state", "0");
    }
  }
  firstTime = false;
}

//---------------------------------------------------------------------------------------------------
void blinkLED (int noOfTimes) {
  for(int i=0; i< noOfTimes; i++) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
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
      client.subscribe("home/computerSwitch/command");
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
void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }

  if (0 == digitalRead(switchPin))
  {
    switchStatus = !switchStatus;
    Serial.println("switch pressed");
    if (switchStatus)
    {
      blinkLED(2);
      client.publish("home/computerSwitch/state", "1");
      digitalWrite(relay, HIGH);
    }
    else
    {
      blinkLED(1);
      client.publish("home/computerSwitch/state", "0");
      digitalWrite(relay, LOW);
    }
  }
 
  delay(500);
  client.loop();
}
