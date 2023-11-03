#define AOUT_PIN 33
//Connect your sensor to GPIO Pin #33.
//Reference the pin layout for Teyleten Robot ESP-WROOM-32 at https://www.amazon.com/gp/product/B08246MCL5/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1 

void setup() {
  Serial.begin(9600);
}

void loop() {
  int value = analogRead(AOUT_PIN);
  //read the analog value from the sensor

  Serial.print("Moisture value: ");
  Serial.println(value);

  delay(1000);
}