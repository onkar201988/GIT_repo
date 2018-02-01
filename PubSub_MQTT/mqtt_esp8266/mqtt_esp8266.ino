#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "LakeViewWiFi";
const char* password = "P@ssLakeView";
const char* mqtt_server = "192.168.2.12";
const char* mqtt_uname = "onkar20";
const char* mqtt_pass = "onkar20";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
const int lightPin = 2;
const int switchPin = 0;
int switchStatus = 0;

void setup() {
  pinMode(lightPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(switchPin, INPUT);     // Initialize pin 0 as input
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

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
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(lightPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    switchStatus = 1;              //update local switch status with MQTT
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(lightPin, HIGH);  // Turn the LED off by making the voltage HIGH
    switchStatus = 0;              //update local switch status with MQTT
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_2nd_client", mqtt_uname, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("temp/out", "hello world");
      // ... and resubscribe
      client.subscribe("temp/test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  if (0 == digitalRead(switchPin))
  {
    switchStatus = !switchStatus;
    Serial.println("switch pressed");
    if (switchStatus)
    {
      client.publish("temp/test", "1");
      digitalWrite(lightPin, LOW);        //incase of MQTT not working, toggle switch can be used
    }
    else
    {
      client.publish("dev/test", "0");
      digitalWrite(lightPin, HIGH);
    }
    delay(250);
  }
  else
  {}
  client.loop();
}
