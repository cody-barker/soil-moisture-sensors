#include <Wire.h>     
#include <HardwareSerial.h>                                              
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define WIFI_SSID //                                                                                      //Network Name
#define WIFI_PASSWORD //                                                                             //Network Password
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"                                                  //InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_TOKEN "u2yNjH485PXUYjijzxo6KqfL0L_KjTegQAI5bDujuZ7U4ulD3TO0fCyxKAAc7khrQgG4BuORdzbxmDt24YOY9A=="     //InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> <select token>)
#define INFLUXDB_ORG "2123b666e96aaaea"                                                                               //InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_BUCKET "Sensors"                                                                                     //InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define TZ_INFO "UTC-7"

const int DryValue = 3880;
const int WetValue = 1965;

int sensor2Pin = A0;
int sensor2Value = 0;
int sensor2Percent = 0;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);              //InfluxDB client instance with preconfigured InfluxCloud certificate

Point sensor("moisturePercent");                                            //Data point

void setup() 
{
  Serial.begin(9600);                                                //Start serial communication
  
  WiFi.mode(WIFI_STA);                                               //Setup wifi connection
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");                                //Connect to WiFi
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  sensor.addTag("device", DEVICE);                                   //Add tag(s) - repeat as required
  sensor.addTag("SSID", WIFI_SSID);

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");                 //Accurate time is necessary for certificate validation and writing in batches

  if (client.validateConnection())                                   //Check server connection
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

void loop()                                                          //Loop function
{
  sensor2Value = analogRead(sensor2Pin);

  sensor2Percent = map(sensor2Value, DryValue, WetValue, 0, 100);
  sensor.clearFields();

  if (sensor2Percent > 100)
  {
    sensor2Percent = 100;
  }
  else if(sensor2Percent <0)
  {
    sensor2Percent = 0;
  }

  Serial.print("Sensor 2: ");
  Serial.print(sensor2Percent );
  Serial.println("%");

  sensor.addField("sensor2Percent", sensor2Percent);

  if (wifiMulti.run() != WL_CONNECTED)                               //Check WiFi connection and reconnect if needed
    Serial.println("Wifi connection lost");

  if (!client.writePoint(sensor))                                    //Write data point
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  esp_deep_sleep(3600000000);     
                                      
}