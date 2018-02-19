#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <ArduinoOTA.h>

#include "user_data.h"

//-------------variable declaration
long lastMsg = 0;
char msg[50];
int value = 0;
const int posOn = 150;
const int posOff = 45;
const int posNormal = 100;
int switchStatus = 0;

Servo s1; //servo 1

//----------------------------------------------------------------------------------
void setup() {
  s1.attach(servo);
  s1.write(posOff);
  s1.detach();
  pinMode(lightPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(switchPin, INPUT);     // Initialize pin 0 as input
  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(lightPin, LOW);
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
    client.publish("home/hallSwitch/state", "1");
  }
  else {
    blinkLED(1);
    switchStatus = 0;              //update local switch status with MQTT
    runServo(posOff);
    client.publish("home/hallSwitch/state", "0");
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
      // Once connected, publish an announcement...
      client.publish("dev/out", "hello world OTA");
      // ... and resubscribe
      client.subscribe("home/hallSwitch/command");
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
      client.publish("home/hallSwitch/state", "1");
      //digitalWrite(lightPin, LOW);        //incase of MQTT not working, toggle switch can be used
    }
    else
    {
      client.publish("home/hallSwitch/state", "0");
      //digitalWrite(lightPin, HIGH);
    }
    delay(250);
  }
  else
  {}
  client.loop();
}
