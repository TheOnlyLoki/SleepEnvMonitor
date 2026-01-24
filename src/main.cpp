#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"
#include "SCD4x_commands.h"
#define SCD41_ADDR 0x62

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastReconnectAttempt = 0;
unsigned long lastMqttUpload = 0;
String clientId = "ESP8266-" + WiFi.macAddress();

void sendCommand(uint16_t command){
    Wire.beginTransmission(SCD4X::I2C_ADDR);
    Wire.write(command >> 8); // High Byte
    Wire.write(command & 0xFF); // Low Byte
    Wire.endTransmission();
}

bool isDataReady() {
    sendCommand(SCD4X::GET_DATA_READY_STATUS);
    delay(1);
    if (Wire.requestFrom(SCD4X::I2C_ADDR, (uint8_t)3) == 3){
        uint16_t status = (uint16_t)Wire.read() << 8 | Wire.read() << 8;
        Wire.read();
        return (status & 0x07FF) != 0; // if last 11 bits are zero, data is not ready
    }
    return false;
}

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

void handle_sensor_data() {
    // Command: READ_MEASUREMENT (0xEC05)
    sendCommand(SCD4X::READ_MEASUREMENT);

    delay(1); // short pause for sensor

    uint8_t bytesReceived = Wire.requestFrom((uint8_t)SCD41_ADDR, (uint8_t)9);
    if(bytesReceived == 9){
        // CO2 (ppm)
        uint16_t co2 = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read(); // skip checksum

        // Temperature (convert to °C)
        uint16_t tempRaw = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read();
        float temp = SCD4X::calculateTemp(tempRaw);

        // Humidity (convert to %)
        uint16_t humRaw = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read();
        float hum = SCD4X::calculateHum(humRaw);



        unsigned long now = millis();
        if (now - lastMqttUpload >= 5000) { 
            lastMqttUpload = now;

            char payload[128];
            snprintf(payload, sizeof(payload),
                    "{\"CO2\": %d, \"temperature\": %.2f, \"humidity\": %.2f}",
                    co2, temp, hum
            );

            client.publish("room/bedroom/sensor", payload);
            // Serial.println(">>> Data published to InfluxDB via MQTT");

            // CSV style print to console
            // Serial.print(millis()); Serial.print(";");
            // Serial.print(co2);      Serial.print(";");
            // Serial.print(temp, 2);  Serial.print(";");
            // Serial.println(hum, 2);
        } else {
            // Optional: Lokales Debugging alle 5 Sek, ohne die DB zu füllen
            Serial.printf("Local Read: CO2: %d ppm | Temp: %.2f%% | Hum: %.2f%%\n", co2, temp, hum);
        }
    }
}

void setup() {
    Serial.begin(115200);
    // SDA to GPIO4 (D2), SCL to GPIO5 (D1)
    Wire.begin(4, 5); 

    // Command: START_PERIODIC_MEASUREMENT (0x21B1)
    sendCommand(SCD4X::START_PERIODIC_MEASUREMENT);
    // Command: SET_AUTOMATIC_SELF_CALIBRATION 
    sendCommand(SCD4X::SET_AUTO_SELF_CALIBRATION_EXE);
    
    setup_wifi(); // connect to WiFi router
    client.setServer(MQTT_BROKER,1883);
    // Serial.println("timestamp_ms;CO2_ppm;Temp_C;Hum_pct"); // CSV style print
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

    if (isDataReady()) {
        handle_sensor_data();
    }

}