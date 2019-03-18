//-----------------------------------------------------------------------
//
// This ecu will help ESP8266 to wake up on interrupt
// and send the door status to server and disable ESP,
// and go to deep sleep forever untill next update
//
//    1. Power ON, Read door switch, and send the current info to server
//    2. Once ESP replies it has completed the data transfer, disable ESP
//    3. Based on current door swict status, setup next interrupt (HIGH or LOW)
//    4. Go to low power
//------------------------------------------------------------------------
#include "LowPower.h"

// Use pin 2 as wake up pin
const int wakeUpPin = 2;
const int ESP_enablePin = 3;
const int ESP_dataPin = 4;
const int ESP_feedbackPin = 5;

void setup()
{
    Serial.begin(115200);
    // Configure wake up pin as input.
    // This will consumes few uA of current.
    pinMode(wakeUpPin, INPUT);
    pinMode(ESP_enablePin, OUTPUT);
    pinMode(ESP_dataPin, OUTPUT);
    pinMode(ESP_feedbackPin, INPUT);
}

void sendDataToEsp()
{
  Serial.println("Just Woke up, lets do some work...!!!");
  
  digitalWrite(ESP_enablePin, HIGH);  //Wake up ESP
  delay(500);                         // Give some time to ESP to connect to WiFi
  digitalWrite(ESP_dataPin, digitalRead(wakeUpPin));  //Read current door status, and send to ESP

  while(LOW == digitalRead(ESP_feedbackPin))
  {
    delay(100);                       // Wait until ESP actually wakes up and send feedback as HIGH
  }
  //Some more delay, why not :-P
  delay(100);

  while(HIGH == digitalRead(ESP_feedbackPin))
  {
    delay(100);                       // Wait untill feedback pin is HIGH, once LOW, exit loop
  }
  
  Serial.println("Feedback came, turning off ESP");
  digitalWrite(ESP_enablePin, LOW);  // Send ESP to Sleep
  digitalWrite(ESP_dataPin, LOW);    // Reset data pin
  delay(200);
}

void loop() 
{
  //Setup interrupt based on current switch status
  if(HIGH == digitalRead(wakeUpPin))
  {
    delay(500);
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(0, sendDataToEsp, FALLING);
  }
  else
  {
    delay(500);
    // Allow wake up pin to trigger interrupt on high.
    attachInterrupt(0, sendDataToEsp, RISING);
  }
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(0); 
  
  Serial.println("Just Finished work, going to sleep");
}
