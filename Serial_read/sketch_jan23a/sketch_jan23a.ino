#include <ESP8266WiFi.h>

String a;
String b = String("Onkar\r\n");
String c = String("Onkar");
const int lightPin = 2;

void setup()
{
  Serial.begin(115200); // opens serial port, sets data rate to 9600 bps
  Serial.println("Type your name");
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, HIGH);
}

void blinkLED (int noOfTimes, int blinkDelay) {
  for(int i=0; i< noOfTimes; i++) {
    digitalWrite(lightPin, LOW);
    delay(blinkDelay);
    digitalWrite(lightPin, HIGH);
    delay(blinkDelay);
  }
}

void readSerialData()
{
  a= Serial.readString();// read the incoming data as string

  if(a == b)
  {
    Serial.println("Matched with b");
    Serial.println(b);
    blinkLED(2, 100);
  }
  else if(a == c)
  {
    Serial.println("Matched with C");
    Serial.println(c);
    blinkLED(3, 100);
  }
  else
  {
    Serial.print("No match found, current string is: ");
    Serial.print(a);
    blinkLED(1, 100);
  }
}
void loop()
{
  if(Serial.available())
  {
    readSerialData();
  }
  else
  {
    blinkLED(5, 200);
  }
}
