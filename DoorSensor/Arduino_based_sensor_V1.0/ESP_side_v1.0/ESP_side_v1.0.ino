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

String readData;

enum sysState {
  INIT,
  AWAKE,
  READ_DATA,
  DONE
};

enum sysState systemState = INIT;

WiFiClient espClient;
PubSubClient client(espClient);

//----------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.print("awake");                  // Send response as ready
  systemState = AWAKE;
}

//----------------------------------------------------------------------------------------------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");      
  //Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected"); 
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

//----------------------------------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_device_name,mqtt_uname,mqtt_pass)) {
      //Serial.println("connected");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//----------------------------------------------------------------------------------------------------
void readSerialData()
{
  readData = Serial.readString();     // read the incoming data as string
  if (String("OPEN") == readData)
  {
    client.publish("home/guestRoomWindow/state", "OPEN");
    systemState = READ_DATA;
  }
  else if (String("CLOSED") == readData)
  {
    client.publish("home/guestRoomWindow/state", "CLOSED");
    systemState = READ_DATA;
  }
  else
  {
    // no valid data found, stay in same state
    Serial.print("wrong_data");
  }
}

//----------------------------------------------------------------------------------------------------
void sendSerialData()
{
  Serial.print("done");                  // Send response as ready
  systemState = DONE;
}

//----------------------------------------------------------------------------------------------------
void stateMachine()
{
  switch (systemState) {
    case INIT:
      // do nothing
      break;
    case AWAKE:
      if(Serial.available() > 0)
      {
        readSerialData();
      }
      else
      {
        // do nothing, wait here.
      }
      break;
    case READ_DATA:
      sendSerialData();
      break;
    case DONE:
      break;

    default:
      systemState = INIT;
      break;
  }
}
//----------------------------------------------------------------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  stateMachine();
  delay(100);
}
