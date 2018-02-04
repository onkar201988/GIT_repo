#include <ESP8266WiFi.h>
#include <PubSubClient.h>
extern "C" {
  #include "user_interface.h"
}

const char* ssid = "LakeViewWiFi";
const char* password = "P@ssLakeView";
const int LED_PIN = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  setup_wifi();
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
  // put your main code here, to run repeatedly:
  delay(5000);
  digitalWrite(LED_PIN, LOW);
  delay(5000);
  digitalWrite(LED_PIN, HIGH);
}
