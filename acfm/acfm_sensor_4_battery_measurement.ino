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
#define TZ_INFO "UTC+7"

const int DryValue = 4025;
const int WetValue = 1920;
const int numSensors = 1;

struct SensorData {
  int pin;
  int value;
  int percent;
};

Point sensor("moisturePercent");

SensorData sensorData[numSensors];

int sensorPins[numSensors] = {A0};

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

float measureBatteryVoltage() {
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt += analogReadMilliVolts(A1); // ADC with correction   
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V
  Vbattf = round(Vbattf * 10.0) / 10.0;
  // Apply correction factor
  // float correctionFactor = 0.04; // Adjust this based on your measurements
  // Vbattf += correctionFactor;

  return Vbattf;
}

int mapVoltageToPercentage(float voltage) {
  int voltageInt = int(voltage * 100); // Convert float to integer with appropriate precision
  if (voltageInt >= 300 && voltageInt < 370) {
    return map(voltageInt, 300, 370, 0, 30);
  } else if (voltageInt >= 370 && voltageInt < 400) {
    return map(voltageInt, 370, 400, 30, 70);
  } else if (voltageInt >= 400 && voltageInt <= 420) {
    return map(voltageInt, 400, 420, 70, 100);
  } else {
    Serial.println("Voltage out of range: " + String(voltage));
    return -1; // Or any suitable default value
  }
}


void setup()
{
  Serial.begin(9600); //keep this at 9600 or the esp32c3 won't load into the bootloader. 
  pinMode(A1, INPUT);  

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
    Serial.print(i + 4);
    Serial.print(": ");
    Serial.print(sensorData[i].percent);
    Serial.println("%");
    Serial.print("Sensor ");
    Serial.print(i + 4);
    Serial.print(": ");
    Serial.print(sensorData[i].value);
    Serial.println("");

    sensor.addField("sensor" + String(i + 4) + "Percent", sensorData[i].percent);
  }
}

void writeDataToInfluxDB()
{
  float batteryVoltage = measureBatteryVoltage();
  float mappedVoltage = mapVoltageToPercentage(batteryVoltage);
  Serial.println("Battery Voltage: " + String(batteryVoltage) + "V");
  Serial.println("Battery Percentage: " + String(mappedVoltage) + "%");

  // Create a point for battery voltage data
  Point battery("batteryVoltage");
  battery.addField("voltage", batteryVoltage);
  battery.addField("mappedVoltage", mappedVoltage); // Add the mapped voltage as a field

  if (!client.writePoint(battery))
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}


bool checkWiFiConnection()
{
  return wifiMulti.run() == WL_CONNECTED;
}
