
#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
#endif

//Luft dht11
#include "DHT.h"
#define DHT_TYPE DHT11

//Feuchtigkeitssensor
const int analogPin = A0; //Analog Pin vom ESP
int e = 0;

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "Connect for Identity Theft"       //WICHTIG CHANGEN !!!!
// WiFi password
#define WIFI_PASSWORD "#KG7P@53!>b"   

#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "q3DGgs--tBBWM006JA7I3rha7fCSXKUwtflVmrG-x4TtPD-knBP4EbKOPH9CTT8kGwwsDzXyFjlMfG6VyxhQyw=="
#define INFLUXDB_ORG "9ed6fb73f20651fe"
#define INFLUXDB_BUCKET "AndroidDB"

// Time zone info
#define TZ_INFO "UTC2"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data points
Point temperature("temperature");
Point humidity("humidity");
Point soil_moisture("soil_moisture");

//Luft DHT11
const int DHT_PIN = 5;
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
//SENSOREN
  //Setup Luft
  dht.begin();

  
  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}


  //Temperatur und Luftfeuchtigkeitssensor 

void loop() {
  //Werte von den Sensoren holen !
  //Luft
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //Erd-Feuchtigkeit
  int e = analogRead(analogPin);
  
  Serial.print("Temperatur: ");
  Serial.print(t);
  Serial.print("&deg;C, Luftfeuchtigkeit: ");
  Serial.print(h);
  Serial.println("%");
  
  // Clear fields for reusing the points
  temperature.clearFields();
  humidity.clearFields();
  soil_moisture.clearFields();

  // Simulate sensor readings (replace with actual sensor readings later)
  //int temp = random(20, 30); // Random temperature between 20°C and 30°C
  //int hum = random(40, 60);  // Random humidity between 40% and 60%
  //int soil = random(1, 10);  // Random soil moisture between 1 and 10

  int temp =  (int)t; // Random temperature between 20°C and 30°C
  int hum =   (int)h;  // Random humidity between 40% and 60%
  int soil =       e;  // Random soil moisture between 1 and 10

  // Add sensor data to points
  temperature.addField("value", temp);
  humidity.addField("value", hum);
  soil_moisture.addField("value", soil);

  // Print what we are exactly writing
  Serial.print("Writing temperature: ");
  Serial.println(temperature.toLineProtocol());
  Serial.print("Writing humidity: ");
  Serial.println(humidity.toLineProtocol());
  Serial.print("Writing soil moisture: ");
  Serial.println(soil_moisture.toLineProtocol());

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Write points to InfluxDB
  if (!client.writePoint(temperature) || !client.writePoint(humidity) || !client.writePoint(soil_moisture)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Waiting 10 seconds");
  delay(10000); // Send data every 10 seconds (adjust as needed)
}
