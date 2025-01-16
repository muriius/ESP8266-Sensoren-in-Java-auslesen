
#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
#endif

// WiFi AP SSID
#define WIFI_SSID "WLAN NAME"
// WiFi password
#define WIFI_PASSWORD "WLAN PASSWORT"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "token"
#define INFLUXDB_ORG "org"
#define INFLUXDB_BUCKET "bucket"

// Time zone info
#define TZ_INFO "UTC2"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data points
Point temperature("temperature");
Point humidity("humidity");
Point soil_moisture("soil_moisture");

//Luft DHT11
#include "DHT.h"
#define DHT_TYPE DHT11
const int DHT_PIN = 5;
DHT dht(DHT_PIN, DHT_TYPE);

//Feuchtigkeitssensor
const int analogPin = A0; //Analog Pin vom ESP
int e = 0;

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

  // Clear fields for reusing the points
  temperature.clearFields();
  humidity.clearFields();
  soil_moisture.clearFields();

  int temp =  (int)t;
  int hum =   (int)h;
  int soil =       e;

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
