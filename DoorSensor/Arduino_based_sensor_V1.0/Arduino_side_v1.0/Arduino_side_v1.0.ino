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

enum sysState {
  INIT,
  AWAKE,
  CHECK_ESP_AWAKE,
  SEND_DATA_TO_ESP,
  WAIT_FOR_ESP,
  READY_TO_SLEEP
};

enum sysState systemState = INIT;

//----------------------------------------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    // Configure wake up pin as input.
    // This will consumes few uA of current.
    pinMode(wakeUpPin, INPUT);

    //Pin to enable ESP
    pinMode(ESP_enablePin, OUTPUT);
    digitalWrite(ESP_enablePin, LOW);
    systemState = INIT;
}

//----------------------------------------------------------------------------------------------------
void pin2_ISR()
{
  detachInterrupt(0);     // System will wake here
  systemState = AWAKE;
  
  digitalWrite(ESP_enablePin, HIGH);      //Wake up ESP
  delay(50);
  delay(500);                             // Give some time to ESP to connect to WiFi
}

//----------------------------------------------------------------------------------------------------
void readyToSleep()
{
  digitalWrite(ESP_enablePin, LOW); 
  
  //Setup interrupt based on current switch status
  if(HIGH == digitalRead(wakeUpPin))
  {
    delay(100);
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(0, pin2_ISR, FALLING);
  }
  else
  {
    delay(100);
    // Allow wake up pin to trigger interrupt on high.
    attachInterrupt(0, pin2_ISR, RISING);
  }
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

//----------------------------------------------------------------------------------------------------
bool checkESPAwake()
{
  bool espAwake = false;
  delay(100);
  
  String readData = Serial.readString();     // read the incoming data as string
//  if (readData.endsWith(String("awake")))
//  {
    espAwake = true;
//  }
  return espAwake;
}

//----------------------------------------------------------------------------------------------------
void resetEsp()
{
  digitalWrite(ESP_enablePin, LOW);  // Reset ESP
  delay(50);
  digitalWrite(ESP_enablePin, HIGH);  // Wake up ESP
}

//----------------------------------------------------------------------------------------------------
void sendDataToEsp()
{
  if(LOW == digitalRead(wakeUpPin))
  {
    Serial.print("CLOSED");
  }
  else if(HIGH == digitalRead(wakeUpPin))
  {
    Serial.print("OPEN");
  }

  systemState = WAIT_FOR_ESP;
}

//----------------------------------------------------------------------------------------------------
void checkEspDone()
{
  String readData = Serial.readString();     // read the incoming data as string
  if (String("done") == readData)
  {
    systemState = READY_TO_SLEEP;
  }
  else if (String("wrong_data") == readData)
  {
    systemState = SEND_DATA_TO_ESP;
  }
}
//----------------------------------------------------------------------------------------------------
void stateMachine()
{
  switch (systemState) {
    case INIT:
      pin2_ISR();   // 1st time execution, 
      break;
      
    case AWAKE:
      systemState = CHECK_ESP_AWAKE;
      break;
      
    case CHECK_ESP_AWAKE:
      if(Serial.available())
      {
        if(checkESPAwake())
        {
          systemState = SEND_DATA_TO_ESP;
        }
        else
        {
          resetEsp();
        }
      }
      else
      {
        // Stay in same state
      }
      break;
      
    case SEND_DATA_TO_ESP:
      sendDataToEsp();
      break;
      
    case WAIT_FOR_ESP:
      if(Serial.available())
      {
        checkEspDone();
      }
      else
      {
        // do nothing, wait here.
      }
      break;
      
    case READY_TO_SLEEP:
      readyToSleep();
      break;
      
    default:
      systemState = INIT;
  }
}

//----------------------------------------------------------------------------------------------------
void loop() 
{
  stateMachine();
  delay(100);
}
