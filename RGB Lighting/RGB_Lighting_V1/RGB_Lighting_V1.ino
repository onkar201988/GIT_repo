#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <ArduinoOTA.h>
extern "C" {
  #include "user_interface.h"
}

//-------------- String declairation--------------------------
const char* ssid = "LakeViewWiFi";
const char* password = "P@ssLakeView";
const char* mqtt_server = "192.168.2.12";
const char* mqtt_uname = "onkar20";
const char* mqtt_pass = "onkar20";
const char* mqtt_device_name = "RGB_light";
const char* ota_device_name = "RGB_light";
const char* ota_password = "onkar20";

//-------------variable declaration
#define serialEnable
const int R = 12;
const int G = 14;
const int B = 4;
long lastMsg = 0;
char msg[50];
int value = 0;
const int lightPin = 2;

int redValueCurrent = 0;
int greenValueCurrent = 0;
int blueValueCurrent = 0;

int redValueNext = 0;
int greenValueNext = 0;
int blueValueNext = 0;

String greenLedVal = "0";
String redLedVal = "0";
String blueLedVal = "0";

WiFiClient espClient;
PubSubClient client(espClient);

//----------------------------------------------------------------------------------
void setup() {
  pinMode(lightPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  #ifdef serialEnable
    Serial.begin(115200);
  #endif
  
  setup_wifi();
  setup_OTA();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(lightPin, LOW);
  analogWrite(R, 0);
  analogWrite(G, 0);
  analogWrite(B, 0);
}

//----------------------------------------------------------------------------------------------------
void setup_OTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(ota_device_name);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)ota_password);

  #ifdef serialEnable
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
  #endif
  ArduinoOTA.begin();
}
//----------------------------------------------------------------------------------------------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //for Light sleep
  WiFi.mode(WIFI_STA);
  wifi_set_sleep_type(LIGHT_SLEEP_T); 
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(lightPin, !digitalRead(lightPin));
  }

  digitalWrite(lightPin, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//----------------------------------------------------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived new [");
  Serial.print(topic);
  Serial.print("] ");
  //for (int i = 0; i < length; i++) {
  //  Serial.print((char)payload[i]);
  //}
  //Serial.println();

  String stringOne = (char*)payload;
  
  Serial.println(stringOne);

  // find first ; in the string
  int firstClosingBracket = stringOne.indexOf(';')+1;

  // find second ; in the string
  int secondOpeningBracket = firstClosingBracket + 1;
  int secondClosingBracket = stringOne.indexOf(';', secondOpeningBracket);

  // find the third ; in the string
  int thirdOpeningBracket = secondClosingBracket + 1;
  int thirdClosingBracket = stringOne.indexOf(';', thirdOpeningBracket);

  // using the locations of ; find values 
  greenLedVal = stringOne.substring(0 , (firstClosingBracket - 1));
  redLedVal = stringOne.substring(firstClosingBracket , secondClosingBracket);
  blueLedVal = stringOne.substring((secondClosingBracket +1) , thirdClosingBracket);

  setColor(redLedVal.toInt()*10.24, greenLedVal.toInt()*10.24, blueLedVal.toInt()*10.24);
}

//----------------------------------------------------------------------------------------------------
void setColor (int red, int green, int blue) {
  if(red >= 0 && red <= 1024)
  {
    redValueNext = red;
  }
  if(green >= 0 && green <= 1024)
  {
    greenValueNext = green;
  }
  if(blue >= 0 && blue <= 1024)
  {
    blueValueNext = blue;
  }                                                                                                                                                                                                                                                                                                                                                                                               
}

//----------------------------------------------------------------------------------------------------
void updateLED() {
  if(redValueNext   != redValueCurrent ||
     greenValueNext != greenValueCurrent ||
     blueValueNext  != blueValueCurrent)
     {
        redValueCurrent = redValueNext;
        greenValueCurrent = greenValueNext;
        blueValueCurrent = blueValueNext;

        analogWrite(R, redValueCurrent);
        analogWrite(G, greenValueCurrent);
        analogWrite(B, blueValueCurrent);

        Serial.println(redValueCurrent);
        Serial.println(greenValueCurrent);
        Serial.println(blueValueCurrent);
     }
}
//----------------------------------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_device_name, mqtt_uname, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe("home/RGB_Light/color");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//----------------------------------------------------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  
  if (!client.connected()) {
    reconnect();
  }
  updateLED();
  //delay(10);
  client.loop();
}
