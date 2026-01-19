#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

#define SCD41_ADDR 0x62

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastReconnectAttempt = 0;
String clientId = "ESP8266-" + WiFi.macAddress();

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA); // Station Mode
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP-Address: ");
    Serial.println(WiFi.localIP());
}

bool connect_mqtt() {
    if(client.connect(clientId.c_str())) {
        Serial.println("MQTT connected");
        return true;
    }
    Serial.print("ERROR, rc=");
    Serial.print(client.state());
    return false;
}

void handle_sensor() {
    // Command: READ_MEASUREMENT (0xEC05)
    Wire.beginTransmission(SCD41_ADDR);
    Wire.write(0xEC); // MSB
    Wire.write(0x05); // LSB
    Wire.endTransmission();
    
    delay(1); // short pause for sensor

    Wire.requestFrom(SCD41_ADDR, 9);
    if(Wire.available() == 9){
        // CO2 (ppm)
        uint16_t co2 = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read(); // skip checksum

        // Temperature (convert to °C)
        uint16_t tempRaw = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read();
        float temp = -45.0 + 175.0 * (float)tempRaw / 65535.0;

        // Humidity (convert to %)
        uint16_t humRaw = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read();
        float hum = 100.0 * (float)humRaw / 65535.0;

        // CSV style print to console
        Serial.print(millis()); Serial.print(";");
        Serial.print(co2);      Serial.print(";");
        Serial.print(temp, 2);  Serial.print(";");
        Serial.println(hum, 2);

        // send to mqtt broker TODO
        char payload[128];
        snprintf(payload, sizeof(payload),
                "{\"CO2\": %d, \"temperature\": %.2f, \"humidity\": %.2f}",
                co2, temp, hum
            );

        client.publish("room/bedroom/sensor",payload);
    }

    delay(5000); // intervall = 5sec
}

void setup() {
    Serial.begin(115200);
    // SDA to GPIO4 (D2), SCL to GPIO5 (D1)
    Wire.begin(4, 5); 

    // Command: START_PERIODIC_MEASUREMENT (0x21B1)
    Wire.beginTransmission(SCD41_ADDR);
    Wire.write(0x21); // MSB
    Wire.write(0xB1); // LSB
    Wire.endTransmission();
    
    setup_wifi(); // connect to WiFi router
    client.setServer(MQTT_BROKER,1883);
    Serial.println("timestamp_ms;CO2_ppm;Temp_C;Hum_pct"); // CSV style print
}

void loop() {
    if (!client.connected()) {
        unsigned long now = millis();
        // try to connect every 5 seconds, to not block rest of code
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (connect_mqtt()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        client.loop();
    }

    handle_sensor();
}