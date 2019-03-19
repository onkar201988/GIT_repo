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
const int R = 12;
const int G = 14;
const int B = 4;
long lastMsg = 0;
char msg[50];
int value = 0;
const int lightPin = 2;
int wait = 10;
int pattern = 0;


int timer = 0;
bool changed = false;

int redValueCurrent = 0;
int greenValueCurrent = 0;
int blueValueCurrent = 0;

int redValueNext = 0;
int greenValueNext = 0;
int blueValueNext = 0;

int oldRed = 0;
int oldGreen = 0;
int oldBlue = 0;
bool onceDone = false;

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

  Serial.begin(115200);
  
  setup_wifi();
  setup_OTA();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
  delay(100);
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

    oldRed = redLedVal.toInt()*5.12;
    oldGreen = greenLedVal.toInt()*5.12;
    oldBlue = blueLedVal.toInt()*5.12;
  }

  if (strcmp(topic,"home/RGB_Light/pattern")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    if ((char)payload[0] == '0'){
      pattern = 0;
    }
    else if ((char)payload[0] == '1'){
      pattern = 1;
    }
    else if ((char)payload[0] == '2'){
      pattern = 2;
    }
    else if ((char)payload[0] == '3'){
      pattern = 3;
    }
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

  changed = false;
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
  if(timer < 100 && !changed) {
    crossFade(512,0,0);   //red
    changed = true;
  }
  else if(100 < timer && timer < 200 && !changed){
    crossFade(512,512,0); //Yellow
    changed = true;
  }
  else if(200 < timer && timer < 300 && !changed) {
    crossFade(0,512,0);   //Green
    changed = true;
  }
  else if(300 < timer && timer < 400 && !changed) {
    crossFade(0,512,512); //Cyan
    changed = true;
  }
  else if(400 < timer && timer < 500 && !changed) {
    crossFade(0,0,512);   //Blue
    changed = true;
  }
  else if(500 < timer && timer < 600 && !changed){
    crossFade(512,0,512); //Magenta
    changed = true;
  }
  else if(700 < timer) {
    timer = 0;
    changed = false;
  }
  else if(timer == 100 || timer == 200 || timer == 300 || timer == 400 || timer == 500 || timer == 600){
    changed = false;
  }
  else {
    return;
  }
}

//----------------------------------------------------------------------------------------------------
void dancePattern()
{
  if(timer < 10 && !changed){
    setColorOutput(512,0,0);
    changed = true;
  }
  else if(10 < timer && timer < 20 && !changed){
    setColorOutput(0,0,0);
    changed = true;
  }
  else if(20 < timer && timer < 30 && !changed){
    setColorOutput(512,512,0);
    changed = true;
  }
  else if(30 < timer && timer < 40 && !changed){
    setColorOutput(0,0,0);
    changed = true;
  }
  else if(40 < timer && timer < 50 && !changed){
    setColorOutput(0,512,0);
    changed = true;
  }
  else if(60 < timer && timer < 70 && !changed){
    setColorOutput(0,0,0);
    changed = true;
  }
  else if(70 < timer && timer < 80 && !changed){
    setColorOutput(0,512,512);
    changed = true;
  }
  else if(90 < timer && timer < 100 && !changed){
    setColorOutput(0,0,0);
    changed = true;
  }
    else if(70 < timer && timer < 80 && !changed){
    setColorOutput(0,0,512);
    changed = true;
  }
  else if(90 < timer && timer < 100 && !changed){
    setColorOutput(0,0,0);
    changed = true;
  }
    else if(70 < timer && timer < 80 && !changed){
    setColorOutput(512,0,512);
    changed = true;
  }
  else if(90 < timer && timer < 100 && !changed){
    setColorOutput(0,0,0);
    changed = true;
  }
  else if(100 < timer){
    changed = false;
  }
    else if(timer == 10 
         || timer == 20 
         || timer == 30 
         || timer == 40 
         || timer == 50 
         || timer == 60
         || timer == 70
         || timer == 80
         || timer == 90){
    changed = false;
  }
  else {
    return;
  }
}

//----------------------------------------------------------------------------------------------------
void soothingPattern()
{
  if(timer < 200 && !changed) {
    crossFade(512,0,0);   //red
    changed = true;
  }
  else if(200 < timer && timer < 400 && !changed){
    crossFade(0,0,0);     //black
    changed = true;
  }
  else if(400 < timer && timer < 600 && !changed){
    crossFade(512,512,0); //Yellow
    changed = true;
  }
  else if(600 < timer && timer < 800 && !changed){
    crossFade(0,0,0);     //black
    changed = true;
  }
  else if(800 < timer && timer < 1000 && !changed){
    crossFade(0,512,0);   //Green
    changed = true;
  }
  else if(1000 < timer && timer < 1200 && !changed){
    crossFade(0,0,0);     //black
    changed = true;
  }
  else if(1200 < timer && timer < 1400 && !changed){
    crossFade(0,512,512); //Cyan
    changed = true;
  }
  else if(1400 < timer && timer < 1600 && !changed){
    crossFade(0,0,0);     //black
    changed = true;
  }
  else if(1600 < timer && timer < 1800 && !changed){
    crossFade(0,0,512);   //Blue
    changed = true;
  }
  else if(1800 < timer && timer < 2000 && !changed){
    crossFade(0,0,0);     //black
    changed = true;
  }
  else if(2000 < timer && timer < 2200 && !changed){
    crossFade(512,0,512); //Magenta
    changed = true;
  }
  else if(2200 < timer && timer < 2400 && !changed){
    crossFade(0,0,0);     //black
    changed = true;
  }
    else if(2400 < timer){
    changed = false;
  }
    else if(timer == 200
         || timer == 400
         || timer == 600
         || timer == 800
         || timer == 1000
         || timer == 1200
         || timer == 1400
         || timer == 1600
         || timer == 1800
         || timer == 2000
         || timer == 2200){
    changed = false;
  }
  else {
    return;
  }
}
//----------------------------------------------------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
    
  if (!client.connected()) {
    reconnect();
  }

  if(pattern < 5)
  {
    switch(pattern){
      case 0:
        if(!onceDone){
          crossFade(oldRed, oldGreen, oldBlue);
          onceDone = true;
        }
        break;
        
      case 1:
        rainbowPattern();
        onceDone = false;
        break;

      case 2:
        dancePattern();
        onceDone = false;
        break;

      case 3:
        soothingPattern();
        onceDone = false;
        break;

      default:
        crossFade(0,0,0);
    }
  }
  //updateLED();
  delay(10);
  timer++;
  client.loop();
}
