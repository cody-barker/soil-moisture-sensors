#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#define WIFI_SSID "meadowrue"
#define WIFI_PASSWORD "ruelovescheese"
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "6B4J_o-hFeXaNkFR3EGmANp6yM0b7TT6GKTqXccCjKqXmK0j7A4cdz2p4C6kCHR-zrdCDUOVrxBPtHTgEF4joQ=="
#define INFLUXDB_ORG "961d11768f0d8862"
#define INFLUXDB_BUCKET "acfmnursery"
#define TZ_INFO "UTC-7"

const int DryValue = 3951;
const int WetValue = 2827;
const int numSensors = 3;

struct SensorData {
  int pin;
  int value;
  int percent;
};

Point sensor("moisturePercent");

SensorData sensorData[numSensors];

int sensorPins[numSensors] = {A0, A1, A2};

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

void setup()
{
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  connectToWiFi();
  connectToInfluxDB();

  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WIFI_SSID);

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
}

void loop()
{
  readMoistureLevel();

  if (!checkWiFiConnection())
    Serial.println("Wifi connection lost");

  writeDataToInfluxDB();

  // esp_deep_sleep(3600000000);
  delay(5000);
}

void connectToWiFi()
{
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
}

void connectToInfluxDB()
{
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  else
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void readMoistureLevel()
{
   for (int i = 0; i < numSensors; i++) {
    sensorData[i].value = analogRead(sensorPins[i]);
    sensorData[i].percent = map(sensorData[i].value, DryValue, WetValue, 0, 100);
    sensorData[i].percent = constrain(sensorData[i].percent, 0, 100); // Ensure percent is within range

    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(sensorData[i].percent);
    Serial.println("%");

    sensor.addField("sensor" + String(i + 1) + "Percent", sensorData[i].percent);
  }
}

void writeDataToInfluxDB()
{
  if (!client.writePoint(sensor))
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

bool checkWiFiConnection()
{
  return wifiMulti.run() == WL_CONNECTED;
}

