#ifndef USER_DATA_H
#define USER_DATA_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#define ssid                "LakeViewWiFi"
#define password            "P@ssLakeView"
#define mqtt_server         "192.168.2.12"
#define mqtt_uname          "onkar20"
#define mqtt_pass           "onkar20"
#define mqtt_device_name    "ESP8266DoorLock"
#define ota_device_name     "OTA_Door_Lock"
#define mqtt_topic_state    "home/doorLock/state"
#define mqtt_topic_command  "home/doorLock/command"
#define ota_password        "onkar20"

#define servo             4
#define switchPin         0
#define lightPin          2

WiFiClient                espClient;
PubSubClient              client(espClient);

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

#endif
