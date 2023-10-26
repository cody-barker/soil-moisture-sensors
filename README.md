# Soil Moisture, Temp and Humidity Sensors
Send soil moisture, temp, and humidity readings to an InfluxDB database, so that you can visualize them with your own Grafana dashboard.

The sensors.ino file in this repo will allow you post sensor readings to an InfluxDB, so that you can visualize those readings with Grafana. Part of it is borrowed from <a href="https://www.youtube.com/@MichaelKlements">Michael Klements</a>, and part of it I customized to handle however many moisture sensors you want to use.

<img src="grafana-dashboard-preview.png" alt="grafana-dashboard-preview">

Check out <a href="https://www.youtube.com/watch?v=7M8MHa6W9w0&t=1041s&ab_channel=MichaelKlements">this video</a> from Michael Klements for help setting up the InfluxDB database, and configuring the Grafana dashboard.

sensors.ino is written in C++/arduino

## Hardware
-<a href="https://www.amazon.com/gp/product/B08246MCL5/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1">
ESP32 WROOOM Development Board</a>
<br>
-<a href="https://www.amazon.com/gp/product/B07SYBSHGX/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1">
High-quality capacitive soil moisture sensors</a>
<br>
-<a href="https://www.ebay.com/itm/354878228028">
A DHT22 Digital Temp and Humidity Sensor</a>
<br>
-<a href="https://www.ebay.com/itm/223086170029?var=521899911923">
a breadboard</a>
<br>
-<a href="https://www.amazon.com/EDGELEC-Breadboard-Optional-Assorted-Multicolored/dp/B07GD2BWPY/ref=sr_1_1_sspa?crid=3LKQVI91JXG5Z&keywords=jumper%2Bwires&qid=1698357369&s=electronics&sprefix=jumper%2Bwire%2Celectronics%2C157&sr=1-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&th=1">
Jumper wires</a>
<br>
-Micro USB cable and wall charger or rechargable battery pack

## For Contributors
If you'd like to contribute to this project, email me at codybarker.or@gmail.com

## Project Status
Unlicensed

## Authors
Cody Barker (with some of the intial DB/Wifi configuration code provided by Michael Klements)