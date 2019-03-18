// **** INCLUDES *****
#include <avr/wdt.h>
#include "LowPower.h"

int counter;
void setup()
{
    // No setup is required for this library
    Serial.begin(115200);                     //serial port to display received data
    Serial.println("WDT...");
    counter = 0;
}

void loop() 
{
    // Enter power down state for 8 s with ADC and BOD module disabled
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
    
    // Do something here
    // Example: Read sensor, data logging, data transmission.
    delay(2000);
}

ISR(WDT_vect)
{
  wdt_disable();
  counter++;
  Serial.println("WDT ISR");
  Serial.println(counter);
  yield();
}

