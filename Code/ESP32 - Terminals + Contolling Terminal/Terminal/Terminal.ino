/*********
  Rui Santos - DS18B20 - Complete project details at https://RandomNerdTutorials.com  
  DHT tutorial - https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
  ESP32 Timer Wake Up from Deep Sleep - https://randomnerdtutorials.com/esp32-timer-wake-up-deep-sleep/
*********/

//Libraries:
#include <DHT.h>;
#include <OneWire.h>
#include <DallasTemperature.h>
#include "PubSubClient.h"
#include "WiFi.h"
#include <esp_sleep.h>
#include <BluetoothSerial.h>
#include <esp_bt.h>
#include <esp_wifi.h>

// DS18B20:
// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// DHT22:
// Digital pin connected to the DHT sensor
#define DHTPIN 14
// DHT Type
#define DHTTYPE DHT22
// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture 
#define sMoist 34

// Variables:
// data array:
const int numOfReadings = 4;
float dataArray [numOfReadings];
String dataArrayS [numOfReadings];
// data is initalized at startup

// Replace with your network credentials
const char* ssid = "Gateway";
const char* password = "terminal";

// MQTT
const char* mqtt_server = "192.168.4.1";            // IP of the MQTT broker
const char* clientID = "terminal1";                 // MQTT client ID
// Topics to "home/terminals/terminal_1/*"
const char* topic_Reading = "home/terminals/terminal_1/reading";

// Authentication:
const char* mqtt_username = "project";              // MQTT username
const char* mqtt_password = "admin";                // MQTT password

WiFiClient wifiClient;

// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient); 

void connectWifi();
void sleepTimer();
void soil_moisture();
void print_wakeup_reason();
void MQTT_String(String reading);
String str_create();
void dataInit();

// Sleep Timer
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  14       /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;


void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);

  // init data
  dataInit();
  
  // Sleep Settings
  ++bootCount;  //Increment boot number and print it every reboot
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();  //Print the wakeup reason for ESP32
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  
  // Start the DS18B20 sensor
  sensors.begin();
  
  // Start the DHT22 sensor
  dht.begin();

  // loop
  readTemp();
  readAirTempAndHumidity();
  soil_moisture();
  printValues();
  connectWifi();
  transmitData();

  // Sleep now
  Serial.println("\nGoing to sleep now");

  // light sleep for 10mins - 15sec
  int i = 0;
  for (i=0;i<40;i++){
    delay(100);
    Serial.print(".");
    esp_light_sleep_start();
  }
  
  // deep sleep for 15sec
  Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - \n");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop() {
  
}

void dataInit(){

  pinMode(34, INPUT);
  
  // DS18B20:
  dataArrayS[0]="sTmp";    // Stores temperature value name
  dataArray[0]=0;          // Stores temperature value in C
  
  // DHT22:
  dataArrayS[1]="aTmp";    // Stores air temperature reading value name
  dataArray[1]=0;          // Stores air temperature value in C
  dataArrayS[2]="aHum";    // Stores air humidity reading value name
  dataArray[2]=0;          // Stores air humidity value in Bar
  
   // Soil Moisture:
   dataArrayS[3]="sMoist";    // Stores moisture level reading name
   dataArray[3]=0;            // Stores moisture level
  
  // New Sensor?
  // dataArrayS[4]="nSnsr";    // Stores value reading name
  // dataArray[4]=0;           // Stores value
}

void connectWifi(){
  btStop();
  esp_bt_controller_disable();
  delay(1000);
  Serial2.println("BT STOP");
  
  if(WiFi.status() != WL_CONNECTED)
    connect_MQTT();
}

void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Connect to the WiFi
  WiFi.begin(ssid,password);
  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // IP Address of the ESP32
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Connect to MQTT Broker
  if (client.connect(clientID, mqtt_username, mqtt_password))
    Serial.println("Connected to MQTT Broker!");
  else
    Serial.println("Connection to MQTT Broker failed...");
}

void transmitData(){
  String reading = str_create();
  MQTT_String(reading);
}

String str_create(){
  String reading = "";
  int i = 0;
  reading += dataArrayS[i] + ",";
  reading += dataArray[i];
  for(i=1;i<numOfReadings;i++){
    reading += "#" + dataArrayS[i] + ",";
    reading += dataArray[i];
  }
  Serial.print("\nReadings:\n"+reading+"\n");
  return reading;
}

void MQTT_String(String reading){
  // PUBLISH to topic = topic_Reading
  client.connect(clientID, mqtt_username, mqtt_password);
  if (client.publish(topic_Reading, reading.c_str() ))
    Serial.println("topic_Reading sent!");
  else {
    Serial.println("topic_Reading failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); 
    client.publish(topic_Reading, reading.c_str() );
  } 
}

void printValues(){
  int i = 0;
  for(i=0;i<numOfReadings;i++){
    Serial.print(dataArrayS[i]);
    Serial.print(" :\t");
    Serial.print(dataArray[i]);
    Serial.print("\n");
  }
  
  Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - \n");
}

void soil_moisture(){
  dataArray[3] = map(map(analogRead(sMoist),2590,840,0,100),50,-45,100,0);
}


void readTemp(){
  sensors.requestTemperatures();
  dataArray[0] = sensors.getTempCByIndex(0);
  /* old values
  temperatureC = sensors.getTempCByIndex(0);
  temperatureF = sensors.getTempFByIndex(0);
  */
}

void readAirTempAndHumidity(){
  dataArray[1] = dht.readTemperature();
  dataArray[2] = dht.readHumidity();
  /* old values
  AirHumidity = dht.readHumidity();
  AirTemperatureC = dht.readTemperature();
  AirTemperatureF = dht.readTemperature(true);
  HeatIndexF = dht.computeHeatIndex(AirTemperatureF, AirHumidity);
  HeatIndexC = dht.computeHeatIndex(AirTemperatureC, AirHumidity, false);
  */
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
