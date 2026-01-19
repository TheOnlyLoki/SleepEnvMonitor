# SleepBetter: DIY Air Quality Monitor
This is a private hobby project developed to monitor sleep environment quality. It represents my first experience with microcontrollers and IoT infrastructure.

## Project Overview
The system tracks CO2 levels, temperature, and humidity using a sensor node in the bedroom. The data is processed and visualized through a local server stack.

## Tech Stack
Hardware: ESP8266 (NodeMCU) and Sensirion SCD41 (Photoacoustic CO2 sensor).

Enclosure: Custom 3D-printed housing designed to isolate the sensor from microcontroller heat. (Work in Progress)

Transport: MQTT (Mosquitto) for lightweight data transmission.

Infrastructure: Docker-based stack running on a Raspberry Pi.

Data Pipeline: Telegraf (Data collector), InfluxDB 2.x (Time-series database), and Grafana (Visualization).

### API Access
You have to create API-keys in InfluxDB to give the other Services access to the Database!

## Data Flow
The ESP8266 reads sensor data via I2C every 5 seconds and publishes a JSON payload to the MQTT Broker.

Telegraf subscribes to the MQTT topic, parses the JSON, and writes the metrics into InfluxDB.

Grafana queries the database to display real-time trends and historical data.

## Access
The web interface can be accessed within the local network:

URL: http://[RASPBERRY_PI_IP]:3000

Dashboard: Navigate to the "Sleep Quality" dashboard to view live CO2, temperature, and humidity metrics.
