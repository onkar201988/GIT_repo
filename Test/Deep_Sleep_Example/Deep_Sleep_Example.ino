#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "LakeViewWiFi";
const char* password = "P@ssLakeView";
const char* mqtt_server = "192.168.2.12";
const char* mqtt_user = "onkar20";
const char* mqtt_pass = "onkar20";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int loopCount = 0;

const int LED_PIN = 2;
const int TEST_GPIO = 13;

void setup() {
  pinMode(LED_PIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(TEST_GPIO, OUTPUT);
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
    digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    digitalWrite(TEST_GPIO, LOW);
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
    digitalWrite(TEST_GPIO, HIGH);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_devboard", mqtt_user, mqtt_pass)) {
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
  client.loop();
  delay(10);
  loopCount++;

  if(loopCount > 20) {
  Serial.println("I'm awake, but I'm going into deep sleep mode for 5 seconds");
  ESP.deepSleep(10000000, WAKE_RF_DEFAULT);
  }
}
