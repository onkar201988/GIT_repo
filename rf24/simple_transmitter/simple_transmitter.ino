#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LowPower.h>

//----------------------------------------------------------------------------------------------------
const uint8_t pinCE = 8;                    //This pin is used to set the nRF24 to standby (0) or active mode (1)
const uint8_t pinCSN = 9;                   //This pin is used for SPI comm chip select
RF24 wirelessSPI(pinCE, pinCSN);            // Declare object from nRF24 library (Create your wireless SPI) 
const uint64_t wAddress = 0xB00B1E50C3LL;   //Create pipe address for the network and notice I spelled boobies because I am mature, the "LL" is for LongLong type
const uint8_t rFChan = 89;                  //Set channel frequency default (chan 84 is 2.484GHz to 2.489GHz)
const uint8_t rDelay = 7;                   //this is based on 250us increments, 0 is 250us so 7 is 2 ms
const uint8_t rNum = 5;                     //number of retries that will be attempted
const int hallSensor = 2;                   //Pin number where hall sensor is connected

bool isDoorWakeup = false;                  // Flag to store if wake up due to door sensor or not
bool doorStatus = false;                    // Door sensor value global
//----------------------------------------------------------------------------------------------------
//Create a structure to hold sensor data and channel data
struct PayLoad {
  uint8_t chan;
  uint8_t sensor;
};

PayLoad payload; //create struct object

//----------------------------------------------------------------------------------------------------
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(hallSensor, INPUT);
  
  wirelessSPI.begin();                      //Start the nRF24 module
  wirelessSPI.setChannel(rFChan);           //set communication frequency channel
  wirelessSPI.setRetries(rDelay,rNum);      //if a transmit fails to reach receiver (no ack packet) then this sets retry attempts and delay between retries   
  wirelessSPI.openWritingPipe(wAddress);    //open writing or transmit pipe
  wirelessSPI.stopListening();              //go into transmit mode
  
  Serial.begin(115200);                     //serial port to display received data
  Serial.println("Network slave is online...");
  payload.chan = 1;
  payload.sensor = 0;
  //setInturrupt();
  //pin2_ISR();
}

//----------------------------------------------------------------------------------------------------
void pin2_ISR()
{
  Serial.println("Just woke up");
  isDoorWakeup = true;
  doorStatus = digitalRead(hallSensor);
  Serial.println(doorStatus);
  payload.sensor = (uint8_t) doorStatus;
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
  wirelessSPI.powerUp();
    delay(50);
    
    if (!wirelessSPI.write(&payload, sizeof(payload))){  //send data and remember it will retry if it fails
      Serial.println("Sending failed, check network");
    }
    else
    {
      Serial.println("Sending successful, data sent");
    }
    
    wirelessSPI.powerDown();
}
//----------------------------------------------------------------------------------------------------
void loop() {

  //if(isDoorWakeup)
  {
    payload.sensor++;
    sendData();
    //setInturrupt();
  }
  //else
  {
    
  }
  //LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  delay(500);
}
