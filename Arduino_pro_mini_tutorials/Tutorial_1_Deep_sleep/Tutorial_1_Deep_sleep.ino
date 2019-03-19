#include "arduino.h"
#include "LowPower.h"

void setup()
{
  pinMode(13,OUTPUT);
  Serial.begin(115200);
}

void loop() 
{
  Serial.println("Start");
  digitalWrite(13,HIGH);
  delay(2000);
  digitalWrite(13,LOW);
  LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
}

