

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "Gateway";
const char* password = "terminal";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.4.1";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// fan pin output
//#define output_fan 32

// fan run period
long lastOn = 0;
int period = 60000;
int flag_on = 0;

void setup() {
  Serial.begin(115200);

  pinMode(32, OUTPUT);
  //ledcAttachPin(output_fan, ledChannel);
 
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  client.connect("controlling", "project", "admin");
  client.subscribe("home/controlling");
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  String ON = "on";
  String OFF = "off";

  if(messageTemp.equals(ON)){
    Serial.print("Fan is on!\n\n");
    digitalWrite(32 ,HIGH);
    flag_on = 1;
    lastOn = millis();
  } else if(messageTemp.equals(OFF)){
    Serial.print("Fan is off!\n\n");
    digitalWrite(32 ,LOW);
  }
}


void timer_off(){
  digitalWrite(32 ,LOW);
  flag_on = 0;
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("controlling", "project", "admin")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("home/controlling");
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

  long now = millis();
  if (now - lastMsg > 5000) 
    lastMsg = now;

  now = millis();
  if ((now - lastOn > period) && flag_on==1)
    timer_off();
    
}
