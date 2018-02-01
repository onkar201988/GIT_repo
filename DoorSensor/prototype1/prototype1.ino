#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
extern "C" {
  #include "user_interface.h"
}

const char* ssid = "LakeViewWiFi";
const char* password = "P@ssLakeView";
const int lightPin = 2;
const int enablePin = 4;
int enableCount = 0;

void setup() {
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  //pinMode(enableInputPin, INPUT);
  
  pinMode(lightPin, OUTPUT);
  Serial.begin(115200);
  setup_wifi();

  
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("OTATestDevice");
                                         
  // No authentication by default
   ArduinoOTA.setPassword((const char *)"onkar20");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");      
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();

  for(int i = 0; i < 3; i++)
  {
    digitalWrite(lightPin, LOW);   // turn the LED on (HIGH is the voltage level)
    Serial.println("ON new"); 
    delay(1000);                       // wait for a second
    digitalWrite(lightPin, HIGH);    // turn the LED off by making the voltage LOW
    Serial.println("OFF new");
    delay(1000);                       // wait for a second
  }

    Serial.println("enable state is low.. going to be disabled");
    delay(1000);
    digitalWrite(enablePin, LOW);

    delay(50);
    enableCount++;

    if(enableCount > 2)
    {
        ESP.deepSleep(10000000, WAKE_RF_DEFAULT);
    }
  
}
