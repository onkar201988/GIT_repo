#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Servo.h>

#define servo             4
#define switchPin         0
bool switchVal = false;
const int posOn = 200;
const int posOff = 0;

void setup() {
  // put your setup code here, to run once:

  pinMode(servo, OUTPUT);
  pinMode(switchPin, INPUT);
}

void runServo(int angle)
{
  
}

void loop() {
  // put your main code here, to run repeatedly:

    if (0 == digitalRead(switchPin))
    {
      switchVal = !SwitchVal;
      if (switchVal)
      {
        runServo(posOn);
        Serial.println("Door unlocked");
      }
      else
      {
        runServo(posOff);
        Serial.println("Door locked");
      }
     delay(250);
    }
}
