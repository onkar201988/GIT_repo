#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LowPower.h>
#include <Vcc.h>
#include <avr/wdt.h>

//#define debug                               // comment this to remove serial prints
//----------------------------------------------------------------------------------------------------
const uint64_t pipeAddress = 0xB00B1E50C3LL;// Create pipe address for the network, "LL" is for LongLong type
const uint8_t rfChannel = 89;               // Set channel frequency default (chan 84 is 2.484GHz to 2.489GHz)
const uint8_t retryDelay = 7;               // this is based on 250us increments, 0 is 250us so 7 is 2 ms
const uint8_t numRetries = 5;               // number of retries that will be attempted
const uint8_t alarmDisableTime = 2;         // Amount of time to disable the alarm when long switch is pressed (8 Sec x Num)

const uint8_t CE_pin = 8;                   // This pin is used to set the nRF24 to standby (0) or active mode (1)
const uint8_t CSN_pin = 9;                  // This pin is used for SPI comm chip select
const uint8_t hallSensor_pin = 2;           // Pin number where hall sensor is connected
const uint8_t button_pin = 3;               // switch connected pin
const uint8_t redLED_pin = 4;               // Red LED pin
const uint8_t greenLED_pin = 5;             // Green LED pin
const uint8_t blueLED_pin = 6;              // Blue LED pin

const float VccMin   = 1.8;                 // Minimum expected Vcc level, in Volts. (0%)
const float VccMax   = 3.2;                 // Maximum expected Vcc level, in Volts. (100%)
const float VccCorrection = 1.0/1.0;        // Measured Vcc by multimeter divided by reported Vcc

const int sleepDuration = 180;              // Sleep duration to report battery (8 Sec x number of times)(180 for a day)
volatile int sleepCounter = 0;              // Counter to keep sleep count

volatile bool disableAlarm = false;         // Flag to disable the alarm (don't send the data)
volatile int alarmDisableCounter = 0;       // This counter is used in wdt isr to detect if waking up after a day 
                                            // or waking up after switch press to disable alarm
                                            
volatile bool doorStatus = false;           // Door sensor value global
volatile byte buttonState = LOW;            // Internal button state
volatile byte buttonStateShort = LOW;       // Final button state for short press
volatile byte buttonStateLong = LOW;        // Final button state for long press
volatile unsigned long buttonHighTime;      // Internal time variable to store start of high time
volatile unsigned long buttonLowTime;       // Internal time varible to store start of low time
//----------------------------------------------------------------------------------------------------
const int SENSORTYPE  = 0;
const int DOORNUMBER  = 1;
const int DOORSTATUS  = 2;
const int BATTERY     = 3;
const int TEPMERATURE = 4;
const int HUMIDITY    = 5;
const int LIGHT       = 6;

const int PAYLOAD_LENGTH = 7;

char send_payload[PAYLOAD_LENGTH];

Vcc vcc(VccCorrection);                     // create VCC object
RF24 rfRadio(CE_pin, CSN_pin);              // Declare object from nRF24 library (Create your wireless SPI) 
//----------------------------------------------------------------------------------------------------
void setup() {

  send_payload[SENSORTYPE]  = 'D';             // Sensor type [D:Door, T:Temerature, etc]
  send_payload[DOORNUMBER]  = '1';             // Sensor number[1: main door, 2:kitchen window, 3:bedroom window, etc]
  send_payload[DOORSTATUS]  = 'C';             // Door sensor status, C:Closed, O:Open]
  send_payload[BATTERY]     = (char) 100;      // Battery status, 0-100%
  send_payload[TEPMERATURE] = (char) 0;        // Temperature
  send_payload[HUMIDITY]    = (char) 0;        // Humidity
  send_payload[LIGHT]       = (char) 0;        // Light intensity
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(hallSensor_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(hallSensor_pin), hallSwitch_ISR, CHANGE);

  pinMode(button_pin, INPUT);
  digitalWrite(button_pin, HIGH);
  attachInterrupt(digitalPinToInterrupt(button_pin), buttonSwitch_ISR, CHANGE);
  
  pinMode(redLED_pin, OUTPUT);
  pinMode(greenLED_pin, OUTPUT);
  pinMode(blueLED_pin, OUTPUT);
  blinkLED(50,50,50,200);
  
  rfRadio.begin();                              //Start the nRF24 module
  rfRadio.setPALevel(RF24_PA_LOW);              //Set low power for RF24
  rfRadio.setChannel(rfChannel);                //set communication frequency channel
  rfRadio.setRetries(retryDelay,numRetries);    //if a transmit fails to reach receiver (no ack packet) then this sets retry attempts and delay between retries   
  rfRadio.openWritingPipe(pipeAddress);         //open writing or transmit pipe
  rfRadio.stopListening();                      //go into transmit mode

  #ifdef debug
    Serial.begin(115200);                       //serial port to display received data
    Serial.println("Door sensor is online...");
  #endif
  
  hallSwitch_ISR();                             // At start, read sensor data
  sendData();                                   // and send to server
}

//----------------------------------------------------------------------------------------------------
void hallSwitch_ISR()
{
  delay(50);
  #ifdef debug
    Serial.println("In Reed switch ISR");
  #endif
  
  doorStatus = digitalRead(hallSensor_pin);
  
  #ifdef debug
    Serial.println(doorStatus);
  #endif

  if(disableAlarm == false) // only update the status when alarm is allowed
  {
    send_payload[DOORSTATUS]  = doorStatus ? 'O':'C';
    sendData();
  }
  else
  {
    if(doorStatus == HIGH)      // if door is opened during alarm disable, then eneble alarm
    {
      disableAlarm = false;     // eneble alarm, so that when door is closed, the data will be sent
      alarmDisableCounter = 0;  // reset disable counter
    }
  }
}

//----------------------------------------------------------------------------------------------------
void buttonSwitch_ISR()
{
  #ifdef debug
    Serial.println("In button switch ISR");
  #endif

  if(digitalRead(button_pin) == LOW)
  {
    buttonLowTime = millis();
    buttonState = HIGH;
  }
  if( (digitalRead(button_pin) == HIGH) && (buttonState == HIGH) )
  {
    buttonHighTime = millis();
    
    if( (buttonHighTime - buttonLowTime) > 30 && (buttonHighTime - buttonLowTime) < 800)
    {
      buttonStateShort = HIGH;
      buttonState = LOW;
    }
    else if ( (buttonHighTime - buttonLowTime) >= 800  && (buttonHighTime - buttonLowTime) < 4000)
    {
      buttonStateLong = HIGH;
      buttonState = LOW;
    }
  }
}

//----------------------------------------------------------------------------------------------------
ISR(WDT_vect)
{
  wdt_disable();
  if(disableAlarm == true)          // Alarm was disables by long switch press
  {
    #ifdef debug
      Serial.println("In WDT ISR, after long switch press");
    #endif
    if(alarmDisableCounter > 0)     // wait for 16 sec, and then enable alarm
    {
      alarmDisableCounter --;
    }
    else
    {
      disableAlarm = false;
    }
  }
  else
  {
    // do nothing, return immediately
  }
}

//----------------------------------------------------------------------------------------------------
void blinkLED(int red, int green, int blue, int duration)
{
  analogWrite(redLED_pin, red);
  analogWrite(greenLED_pin, green);
  analogWrite(blueLED_pin, blue);

  delay(duration); // Blink LEDs

  analogWrite(redLED_pin, 0);
  analogWrite(greenLED_pin, 0);
  analogWrite(blueLED_pin, 0);
}

//----------------------------------------------------------------------------------------------------
void showBattery()
{
  if(send_payload[BATTERY] >= 65)
  {
    blinkLED(0, 100, 0, 1000); // Show LED Green
  }
  else if((send_payload[BATTERY] < 65) && (send_payload[BATTERY] >= 25))
  {
    blinkLED(70, 70, 0, 1000); // Show LED Yellow
  }
  else if(send_payload[BATTERY] < 25)
  {
    blinkLED(100, 0, 0, 1000); // Show LED RED
  }
}

//----------------------------------------------------------------------------------------------------
void sendData()
{
  rfRadio.powerUp();
  delay(50);
  if (!rfRadio.write(send_payload, PAYLOAD_LENGTH))
  #ifdef debug
    {  //send data and remember it will retry if it fails
      Serial.println("Sending failed, check network");
    }
    else
    {
      Serial.println("Sending successful, data sent");
    }
  #else
    {
    }
  #endif

  rfRadio.powerDown();
}

//----------------------------------------------------------------------------------------------------
void loop()
{
  // If Button is pressed for short duration, then show battery life
  if(buttonStateShort == HIGH)
  {
    #ifdef debug
      Serial.println("Short press detected...");
    #endif
    //showBattery();
    blinkLED(0, 100, 0, 1000);
    buttonStateShort = LOW;
  }
  // If button is pressed for long duraion, then disable alarm (don't send data)
  else if(buttonStateLong == HIGH)
  {
    #ifdef debug
      Serial.println("Long press detected...");
    #endif
    blinkLED(0, 0, 100, 1000);
    disableAlarm = true;
    alarmDisableCounter = alarmDisableTime;    // Wait for 2x8=16 sec
    buttonStateLong = LOW;
  }
  else if(buttonState == LOW)
  { // If switch is not pressed, then only continue sleeping
    #ifdef debug
      Serial.println("No button press, going to sleep again..");
    #endif
    if((sleepCounter > sleepDuration) && disableAlarm == false )
    {
      send_payload[BATTERY] = (char) vcc.Read_Perc(VccMin, VccMax);
      sendData();
      sleepCounter = 0;
      delay(50);
    }
    else
    {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
    sleepCounter++;
  }
}
