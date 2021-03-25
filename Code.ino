
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHTesp.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>
//Ports de commande du moteur B
#define motorPin1 0
#define motorPin2 2
#define enablePin 4


#define WLAN_SSID "Livebox-8B10" // Identifiants WiFi
#define WLAN_PASS "6FbrkY5qKhjQkywm79"

/*********************Adafruit Set Up******************/
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "malikotte"
#define AIO_KEY "aio_wHEX82Lhi6cVFDHQTMMchcWxwWsD"
#define MY_BLUE_LED_PIN 1

//WIFI CLIENT
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe Bool1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/servp");

void MQTT_connect();

Servo servo;
DHTesp dht;
//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.1.18:1880/update-sensor";
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 1000;


void setup() {
  
  Serial.begin(9600);
  servo.attach(2);
  servo.write(0);
  delay(2000);
  
 
  //Connect to WIFI
  Serial.println();Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");    
  }
  Serial.println();

  Serial.println("WiFi Connected");
  Serial.println("IP adress: ");
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&Bool1);
   Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);
  dht.setup(5, DHTesp::DHT22); // Connect DHT sensor to GPIO 17

 
  
}

void loop() {
  MQTT_connect();
  delay(dht.getMinimumSamplingPeriod());
float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  delay(2000);
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");
      char json[255];
      sprintf(json,"{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"%f\",\"value2\":\"%f\",\"value3\":\"49.54\"}",temperature,humidity);
//      char json[]="{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"temperature\",\"value2\":\"49.54\",\"value3\":\"49.54\"}";

      int httpResponseCode = http.POST(json);
      http.writeToStream(&Serial);

      Serial.print("httpResponseCode: ");
    Serial.println(httpResponseCode);
     
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  Adafruit_MQTT_Subscribe *subscription;
  while((subscription = mqtt.readSubscription(20000))){
    if (subscription == &Bool1){
      Serial.print(F("Got: "));
      Serial.println((char *)Bool1.lastread);
      int Bool1_State = atoi((char *)Bool1.lastread); 
      if (Bool1_State == 5){
        digitalWrite(MY_BLUE_LED_PIN, LOW);
        Serial.println("ON");
        servo.write(90);
        delay(100);
        servo.write(0);
      } else {
        Serial.println("OFF");
        servo.write(0);
      }
    }
  }

}

void MQTT_connect() {

  int8_t ret;

  if (mqtt.connected()){
    return;
  }

  Serial.println("Connecting to MQTT...");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT Connection in 5 seconds...");
    delay(5000);
    retries--;
    if (retries == 0) {
      while(1);
    }
    
  }
  Serial.println("MQTT COnnected");
  


  
}
