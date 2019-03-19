/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network 
 *
 * TRANSMITTER NODE
 * Every 2 seconds, send a payload to the receiver node.
 */
#include <LowPower.h>
//#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(7,8);                    // nRF24L01(+) radio attached using Getting Started board 

//RF24Network network(radio);          // Network uses that radio

byte addresses[][6] = {"1Node","2Node"};

unsigned long packets_sent;          // How many have we sent already

struct payload_t {                  // Structure of our payload
  unsigned long ms;
  unsigned long counter;
};

void setup(void)
{
  Serial.begin(115200);
  Serial.println("RF24Network/examples/helloworld_tx/");
 
  SPI.begin();
  delay(500);
  
  radio.begin();
  radio.setChannel(115);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  
  //radio.startListening();
  radio.stopListening();
}

void loop() {
    
  Serial.print("Sending...");
  payload_t payload = { millis(), packets_sent++ };

  //radio.stopListening();
  bool ok = radio.write(&payload,sizeof(payload));
  if (ok){
    Serial.println("ok.");
  }
  else {
    Serial.println("failed.");
  }

  //radio.startListening();
  
  delay(500);
  radio.powerDown();
  
  delay(500);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //delay(8000);
  delay(200);
  radio.powerUp();
}

