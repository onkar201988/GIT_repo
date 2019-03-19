#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LowPower.h>
#include <Vcc.h>
#include <avr/wdt.h>

#define debug                               // comment this to remove serial prints
//----------------------------------------------------------------------------------------------------
const uint8_t pinCE = 8;                    // This pin is used to set the nRF24 to standby (0) or active mode (1)
const uint8_t pinCSN = 9;                   // This pin is used for SPI comm chip select
RF24 wirelessSPI(pinCE, pinCSN);            // Declare object from nRF24 library (Create your wireless SPI) 
const uint64_t wAddress = 0xB00B1E50C3LL;   // Create pipe address for the network, "LL" is for LongLong type
const uint8_t rFChan = 89;                  // Set channel frequency default (chan 84 is 2.484GHz to 2.489GHz)
const uint8_t rDelay = 7;                   // this is based on 250us increments, 0 is 250us so 7 is 2 ms
const uint8_t rNum = 5;                     // number of retries that will be attempted

const int hallSensor = 2;                   // Pin number where hall sensor is connected
const int button_pin = 3;                   // switch connected pin
const int redLED_pin = 4;
const int greenLED_pin = 5;
const int blueLED_pin = 6;

const float VccMin   = 1.8;                 // Minimum expected Vcc level, in Volts.
const float VccMax   = 3.2;                 // Maximum expected Vcc level, in Volts.
const float VccCorrection = 1.0/1.0;        // Measured Vcc by multimeter divided by reported Vcc

bool doorStatus = false;                    // Door sensor value global
const int sleepDuration = 2;                // Sleep duration to report battery (8 Sec x number of times)(180 for a day)
int sleepCounter = 0;                       // Counter to keep sleep count

const unsigned long maxSwitchTime = 2000;   // Max time untill long swicth pressed
bool disableAlarm = false;
int alarmDisableCounter = 0;                // This counter is used in wdt isr to detect if waking up after a day 
                                            // or waking up after switch press to disable alarm
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

//----------------------------------------------------------------------------------------------------
Vcc vcc(VccCorrection);                     // create VCC object

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

  pinMode(hallSensor, INPUT);

  pinMode(button_pin, INPUT);
  digitalWrite(button_pin, HIGH);
  attachInterrupt(1, pin3_ISR, FALLING);
  
  pinMode(redLED_pin, OUTPUT);
  pinMode(greenLED_pin, OUTPUT);
  pinMode(blueLED_pin, OUTPUT);
  blinkLED(50,50,50,200);
  
  wirelessSPI.begin();                      //Start the nRF24 module
  wirelessSPI.setPALevel(RF24_PA_LOW);      //Set low power for RF24
  wirelessSPI.setChannel(rFChan);           //set communication frequency channel
  
  wirelessSPI.setRetries(rDelay,rNum);      //if a transmit fails to reach receiver (no ack packet) then this sets retry attempts and delay between retries   
  wirelessSPI.openWritingPipe(wAddress);    //open writing or transmit pipe
  wirelessSPI.stopListening();              //go into transmit mode

  #ifdef debug
    Serial.begin(115200);                     //serial port to display received data
    Serial.println("Door sensor is online...");
  #endif
  
  pin2_ISR();                               // At start, read sensor data, and send to server
  setInturrupt();                           // Set interrupt
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
void pin2_ISR()
{
  delay(50);
  #ifdef debug
    Serial.println("In Reed switch ISR");
  #endif
  
  doorStatus = digitalRead(hallSensor);
  
  #ifdef debug
    Serial.println(doorStatus);
  #endif

  if(disableAlarm == false) // only update the status when alarm is allowed
  {
    send_payload[DOORSTATUS]  = doorStatus ? 'O':'C';
    //send_payload[BATTERY]     = (char) vcc.Read_Perc(VccMin, VccMax);
    sendData();
  }
  setInturrupt();
  //sleepCounter = 0;       // Need to decide what to do with this
}

//----------------------------------------------------------------------------------------------------
void pin3_ISR()
{
  delay(50);
  #ifdef debug
    Serial.println("In button switch ISR");
  #endif
  
  unsigned long beginTime;
  unsigned long endTime;
  bool isLongPressed = false;
  beginTime = millis();
  
  while(digitalRead(button_pin) == false)       // Blocking long press logic
  {
    if((millis()- beginTime) > maxSwitchTime)   // check if the button is still pressed for more than 2 sec
    {
      isLongPressed = true;                     //Long press logic
      disableAlarm = true;
      alarmDisableCounter = 2;
    }
    delay(50);
  }

  if(isLongPressed == false)                    // if swicth was released before 2 sec
  {                                             // short press logic, show battery level
    if(send_payload[BATTERY] >= 65)
    {
      blinkLED(0, 100, 0, 1000); // Show LED Green
    }
    else if( (send_payload[BATTERY] < 65) && (send_payload[BATTERY] >= 25))
    {
      blinkLED(70, 70, 0, 1000); // Show LED Yellow
    }
    else if(send_payload[BATTERY] < 25)
    {
      blinkLED(100, 0, 0, 1000); // Show LED RED
    }
  }
  // Done, return to loop
}

//----------------------------------------------------------------------------------------------------
ISR(WDT_vect)
{
  wdt_disable();
  if(disableAlarm == true)          // Alarm was disables by long switch press
  {
    #ifdef debug
      Serial.println("In WDT ISR, after long swict press");
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
void setInturrupt()
{  
  //Setup interrupt based on current switch status
  if(HIGH == doorStatus)
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
}

//----------------------------------------------------------------------------------------------------
void sendData()
{
  //wirelessSPI.powerUp();
    delay(50);
    if (!wirelessSPI.write(send_payload, PAYLOAD_LENGTH))
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
    
    //wirelessSPI.powerDown();
}
//----------------------------------------------------------------------------------------------------
void loop() {
  if(sleepCounter > sleepDuration)
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
