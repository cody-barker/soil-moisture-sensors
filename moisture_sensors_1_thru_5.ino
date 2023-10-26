#include <Wire.h>
#include "DHT.h"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

#define DEVICE "ESP32"
#define WIFI_SSID //wifi network name
#define WIFI_PASSWORD //wifi network password
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com" //this works for USA
#define INFLUXDB_TOKEN //influx db token
#define INFLUXDB_ORG //influx db organization code
#define INFLUXDB_BUCKET "Sensors" //this can be whatever you named your bucket
#define TZ_INFO "UTC-7" //this can be adjusted to your timezone

DHT dht(32,DHT22);   

const int DryValue = 3560; //this calibration value is based on my personal testing
const int WetValue = 1662; //this calibration value is based on my personal testing
//variables to store sensor readings
int temp = 0;
int humid = 0;
int tempF = 0;

class Sensor {
public:
    int pin;
    int percent;

    Sensor(int _pin) : pin(_pin), percent(0) {
        pinMode(pin, INPUT);
    }

    int readPercent() {
        int value = analogRead(pin);
        //map the values from calibration to a 1..100 scale
        return map(value, DryValue, WetValue, 0, 100);
    }
};

//the following GPIO pin numbers might differ based on your selections. They must be ADC2 pins to work with wifi.
Sensor sensors[] = {Sensor(33), Sensor(34), Sensor(35), Sensor(39), Sensor(36)};
const int numSensors = sizeof(sensors) / sizeof(sensors[0]);

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point dataPoint("moisturePercent");

void setup() {
    Serial.begin(9600);

    dht.begin();  

    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    dataPoint.addTag("device", DEVICE);
    dataPoint.addTag("SSID", WIFI_SSID);

    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

    if (client.validateConnection()) {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(client.getServerUrl());
    } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
    }
}

void loop() {
    //take temp and humidity readings
    temp = dht.readTemperature();
    humid = dht.readHumidity();
    //convert temp from C to F
    tempF = (temp * 1.8) + 32;

    //store temp and humidity readings in points
    dataPoint.addField("temperature", tempF);
    dataPoint.addField("humidity", humid);

    //display readings in serial monitor of Arduino IDE
    Serial.print("Temp C: ");
    Serial.println(temp);
    Serial.print("Temp F: ");
    Serial.println(tempF);
    Serial.print("Humidity: ");
    Serial.println(humid);

    int sensorPercent[numSensors];

    //take soil moisture readings
    for (int i = 0; i < numSensors; ++i) {
        sensorPercent[i] = sensors[i].readPercent();
    }

    //add bounds to mapped values in event they are <0% or >100%
    for (int i = 0; i < numSensors; ++i) {
        if (sensorPercent[i] > 100) {
            sensorPercent[i] = 100;
        } else if (sensorPercent[i] < 0) {
            sensorPercent[i] = 0;
        }

        //print the sensor reading as a soil moisture percentage
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(sensorPercent[i]);
        Serial.println("%");

        //store each sensor's moisture percentage reading in a point
        dataPoint.addField("sensor" + String(i + 1) + "Percent", sensorPercent[i]);
    }

    if (wifiMulti.run() != WL_CONNECTED) {
        Serial.println("Wifi connection lost");
    }

    if (!client.writePoint(dataPoint)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }

    //use ESP32's deep sleep feature to save power. Argument in microseconds.
    //Default to 1 reading per hour since moisture percentage changes slowly.
    esp_deep_sleep(3600000000);  
}