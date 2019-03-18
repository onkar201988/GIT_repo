#include <ESP8266WiFi.h>
#include <PubSubClient.h>
extern "C" {
  #include "user_interface.h"
}

//----------------------------------------------------------------------------------------------------
const char* ssid              = "LakeViewWiFi";
const char* password          = "P@ssLakeView";
const char* mqtt_server       = "192.168.2.12";
const char* mqtt_uname        = "onkar20";
const char* mqtt_pass         = "onkar20";
const char* mqtt_device_name  = "ESP8266WindowOne";

const int ESP_dataPin         = 12;
const int ESP_feedbackPin     = 14;

WiFiClient espClient;
PubSubClient client(espClient);

//----------------------------------------------------------------------------------------------------
void setup() {
  pinMode(ESP_feedbackPin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  pinMode(ESP_dataPin, INPUT);
  
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

//----------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_device_name,mqtt_uname,mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("home/guestRoomWindow/state", "OPEN");
      // ... and resubscribe
      client.subscribe("home/guestRoomWindow/command");
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
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived new [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("going to be disabled");
    delay(100);
    digitalWrite(enablePin, LOW);
  } 
  else {
    Serial.println("Still waiting for reply");
  }
}

//----------------------------------------------------------------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  delay(500);

}
