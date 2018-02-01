#include <CapacitiveSensor.h>
#include <ESP8266WiFi.h>

// NodeMCU Digital Pins
#define D0  16
#define D1  5  // I2C Bus SCL (clock)
#define D2  4  // I2C Bus SDA (data)
#define D3  0
#define D4  2  // Same as "LED_BUILTIN", but inverted logic
#define D5  14 // SPI Bus SCK (clock)
#define D6  12 // SPI Bus MISO 
#define D7  13 // SPI Bus MOSI
#define D8  15 // SPI Bus SS (CS)
#define D9  3  // RX0 (Serial console)
#define D10 1  // TX0 (Serial console)

#define SEND D6
#define RECEIVE D7

#define lightPin D4

bool flag = false;
 CapacitiveSensor sensor = CapacitiveSensor(SEND, RECEIVE);
 
void setup()
{
  Serial.begin(115200);
  pinMode(lightPin, OUTPUT);
}

void loop()
{
    long start = millis();
    long total = sensor.capacitiveSensorRaw(30);
 
    Serial.print(millis() - start);   
    Serial.print("\t");                 
    Serial.print(total);
    Serial.println();

    if(total > 130 && !flag)
    {
      Serial.print("touched");
      digitalWrite(lightPin, !digitalRead(lightPin));
      flag = true;
    }
    else if(total <130)
    {
      Serial.print("not touched");
      flag = false;
    }
 
    delay(100);
}
