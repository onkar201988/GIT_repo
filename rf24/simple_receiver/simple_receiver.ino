#include <SPI.h>
#include <nRF24L01.h> 
#include <RF24.h> 

const uint8_t pinCE = 8;                    //This pin is used to set the nRF24 to standby (0) or active mode (1)
const uint8_t pinCSN = 9;                   //This pin is used for SPI comm chip select
RF24 wirelessSPI(pinCE, pinCSN);            // Declare object from nRF24 library (Create your wireless SPI) 
const uint64_t rAddress = 0xB00B1E50C3LL;   //Create pipe address for the network and notice I spelled boobies because I am mature, the "LL" is for LongLong type
const uint8_t rFChan = 89;                  //Set channel frequency default (chan 84 is 2.484GHz to 2.489GHz)

//Create a structure to hold fake sensor data and channel data
struct PayLoad {
  uint8_t chan;
  uint8_t sensor;
  uint8_t battery;
};

PayLoad payload; //create struct object

void setup() {
  wirelessSPI.begin();                      //Start the nRF24 module
  wirelessSPI.setChannel(rFChan);           //set communication frequency channel
  wirelessSPI.openReadingPipe(1,rAddress);  //This is receiver or master so we need to be ready to read data from transmitters
  wirelessSPI.startListening();             // Start listening for messages
  Serial.begin(115200);                     //serial port to display received data
  Serial.println("Network master is online...");

}

void loop() {
  if(wirelessSPI.available()){  //Check if recieved data
     wirelessSPI.read(&payload, sizeof(payload)); //read packet of data and store it in struct object
     Serial.print("Received data packet from node: ");
     Serial.println(payload.chan); //print node number or channel
     Serial.print("Node sensor value is: ");
     Serial.println(payload.sensor); //print node's sensor value
     Serial.print("Battery life is: ");
     Serial.println(payload.battery); //print node's sensor value
     Serial.println(); 
  }
}
