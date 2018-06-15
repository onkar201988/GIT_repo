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
int wait = 10;
int pattern = 0;

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

  if (strcmp(topic,"home/RGB_Light/color")==0)
  {
    pattern = 0;
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
  
    //setColor(redLedVal.toInt()*10.24, greenLedVal.toInt()*10.24, blueLedVal.toInt()*10.24);
    crossFade(redLedVal.toInt()*5.12, greenLedVal.toInt()*5.12, blueLedVal.toInt()*5.12);
  }

  if (strcmp(topic,"home/RGB_Light/pattern")==0)
  {
    pattern = (char)payload[0];
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
      client.subscribe("home/RGB_Light/pattern");
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
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
* 
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10, 
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or 
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
* 
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
* 
* The red rises from 0 to 10 in ten steps, the green from 
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
* 
* In the real program, the color percentages are converted to 
* 0-255 values, and there are 1020 steps (255*4).
* 
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(), 
* which calculates the absolute gap between the start and end values, 
* and then divides that gap by 1020 to determine the size of the step  
* between adjustments in the value.
*/

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1. 
*  (R, G, and B are each calculated separately.)
*/

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 511) {
    val = 511;
  } 
  else if (val < 0) {
    val = 0;
  }
  return val;
}

/* crossFade() converts the percentage colors to a 
*  0-255 range, then loops 1020 times, checking to see if  
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

void crossFade(int red, int green, int blue) {

  int stepR = calculateStep(redValueCurrent, red);
  int stepG = calculateStep(greenValueCurrent, green); 
  int stepB = calculateStep(blueValueCurrent, blue);

  for (int i = 0; i <= 1020; i++) {
    redValueNext = calculateVal(stepR, redValueNext, i);
    greenValueNext = calculateVal(stepG, greenValueNext, i);
    blueValueNext = calculateVal(stepB, blueValueNext, i);

    setColorOutput(redValueNext, greenValueNext, blueValueNext);

    delay(1); // Pause for 'wait' milliseconds before resuming the loop

  }
  // Update current values for next loop

  redValueCurrent = redValueNext;
  greenValueCurrent = greenValueNext;
  blueValueCurrent = blueValueNext;
}

void setColorOutput(int redValue, int greenValue, int blueValue)
{
    analogWrite(R, redValue*2);   // Write current values to LED pins
    analogWrite(G, greenValue*2);      
    analogWrite(B, blueValue*2);
}
//----------------------------------------------------------------------------------------------------
void rainbowPattern()
{
  crossFade(512,0,0);   //red
  delay(1000);
  crossFade(512,512,0); //Yellow
  delay(1000);
  crossFade(0,512,0);   //Green
  delay(1000);
  crossFade(0,512,512); //Cyan
  delay(1000);
  crossFade(0,0,512);   //Blue
  delay(1000);
  crossFade(512,0,512); //Magenta
  delay(1000);
}

//----------------------------------------------------------------------------------------------------
void dancePattern()
{
  setColorOutput(512,0,0);
  delay(100);
  setColorOutput(512,512,0);
  delay(100);
  setColorOutput(0,512,0);
  delay(100);
  setColorOutput(0,512,512);
  delay(100);
  setColorOutput(0,0,512);
  delay(100);
  setColorOutput(512,0,512);
  delay(100);
}

//----------------------------------------------------------------------------------------------------
void soothingPattern()
{
  crossFade(512,0,0);   //red
  delay(2000);
  crossFade(0,0,0);     //black
  delay(2000);
  crossFade(512,512,0); //Yellow
  delay(2000);
  crossFade(0,0,0);     //black
  delay(2000);
  crossFade(0,512,0);   //Green
  delay(2000);
  crossFade(0,0,0);     //black
  delay(2000);
  crossFade(0,512,512); //Cyan
  delay(2000);
  crossFade(0,0,0);     //black
  delay(2000);
  crossFade(0,0,512);   //Blue
  delay(2000);
  crossFade(0,0,0);     //black
  delay(2000);
  crossFade(512,0,512); //Magenta
  delay(2000);
  crossFade(0,0,0);     //black
  delay(2000);
}
//----------------------------------------------------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  
  if (!client.connected()) {
    reconnect();
  }

  if(pattern != 0)
  {
    switch(pattern){
      case 1:
        rainbowPattern();
        break;

      case 2:
        dancePattern();
        break;

      case 3:
        soothingPattern();
        break;

      //default:
        //crossFade(0,0,0);
    }
  }
  //updateLED();
  //delay(10);
  client.loop();
}
