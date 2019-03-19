#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LowPower.h>
#include <Vcc.h>

#define debug                               // comment this to remove serial prints
//----------------------------------------------------------------------------------------------------
const uint8_t pinCE = 8;                    // This pin is used to set the nRF24 to standby (0) or active mode (1)
const uint8_t pinCSN = 9;                   // This pin is used for SPI comm chip select
RF24 wirelessSPI(pinCE, pinCSN);            // Declare object from nRF24 library (Create your wireless SPI) 
const uint64_t wAddress = 0xB00B1E50C3LL;   // Create pipe address for the network and notice I spelled boobies because I am mature, the "LL" is for LongLong type
const uint8_t rFChan = 89;                  // Set channel frequency default (chan 84 is 2.484GHz to 2.489GHz)
const uint8_t sensorChannel = 1;            // Sensor number in the network, (1, 2, 3...)
const uint8_t rDelay = 7;                   // this is based on 250us increments, 0 is 250us so 7 is 2 ms
const uint8_t rNum = 5;                     // number of retries that will be attempted
const int hallSensor = 2;                   // Pin number where hall sensor is connected

const float VccMin   = 0.0;                 // Minimum expected Vcc level, in Volts.
const float VccMax   = 3.0;                 // Maximum expected Vcc level, in Volts.
const float VccCorrection = 1.0/1.0;        // Measured Vcc by multimeter divided by reported Vcc

bool doorStatus = false;                    // Door sensor value global
const int sleepDuration = 2;                // Sleep duration to report battery (8 Sec x number of times)(180 for a day)
int sleepCounter = 0;                       // Counter to keep sleep count
//----------------------------------------------------------------------------------------------------
//Create a structure to hold sensor data and channel data
struct PayLoad {
  uint8_t chan;
  uint8_t sensor;
  uint8_t battery;
};

//----------------------------------------------------------------------------------------------------
PayLoad payload;                            // Create struct object
Vcc vcc(VccCorrection);                     // create VCC object

//----------------------------------------------------------------------------------------------------
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(hallSensor, INPUT);
  
  wirelessSPI.begin();                      //Start the nRF24 module
  wirelessSPI.setChannel(89);           //set communication frequency channel

  //wirelessSPI.enableDynamicPayloads();
  
  wirelessSPI.setRetries(rDelay,rNum);      //if a transmit fails to reach receiver (no ack packet) then this sets retry attempts and delay between retries   
  wirelessSPI.openWritingPipe(wAddress);    //open writing or transmit pipe
  wirelessSPI.stopListening();              //go into transmit mode

  #ifdef debug
    Serial.begin(115200);                     //serial port to display received data
    Serial.println("Network slave is online...");
  #endif
  
  payload.chan = sensorChannel;             // Set the channel for current node
  pin2_ISR();                               // At start, read sensor data, and send to server
  setInturrupt();                           // Set interrupt
}

//----------------------------------------------------------------------------------------------------
void pin2_ISR()
{
  delay(100);
  #ifdef debug
    Serial.println("Just woke up");
  #endif
  
  doorStatus = digitalRead(hallSensor);
  
  #ifdef debug
    Serial.println(doorStatus);
  #endif
  
  payload.sensor = (uint8_t) doorStatus;
  payload.battery = (int) vcc.Read_Perc(VccMin, VccMax);
  sendData();
  setInturrupt();
  sleepCounter = 0;
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
    static char send_payload[7];// = "0,1,2,3";
    send_payload[0] = (char) 48;
    send_payload[1] = (char) 49;
    send_payload[2] = (char) 01;
    send_payload[3] = (char) 100;
    send_payload[4] = 'O';
    if (!wirelessSPI.write(send_payload, sizeof(send_payload)))
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
    payload.battery = (int) vcc.Read_Perc(VccMin, VccMax);
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
