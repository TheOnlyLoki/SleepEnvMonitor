#include <Wire.h>
#include <Arduino.h>

#define SCD41_ADDR 0x62

void setup() {
    Serial.begin(115200);
    // SDA an GPIO4 (D2), SCL an GPIO5 (D1)
    Wire.begin(4, 5); 

    // Befehl: START_PERIODIC_MEASUREMENT (0x21B1)
    Wire.beginTransmission(SCD41_ADDR);
    Wire.write(0x21); // MSB
    Wire.write(0xB1); // LSB
    Wire.endTransmission();
    
    Serial.println("Zeitstempel_ms;CO2_ppm;Temp_C;Hum_pct"); // CSV Header
}

void loop() {
    // Befehl: READ_MEASUREMENT (0xEC05)
    Wire.beginTransmission(SCD41_ADDR);
    Wire.write(0xEC); // MSB
    Wire.write(0x05); // LSB
    Wire.endTransmission();
    
    delay(1); // Kurze Pause für den Sensor

    Wire.requestFrom(SCD41_ADDR, 9);
    if(Wire.available() == 9){
        // 1. CO2 (Direkt in ppm)
        uint16_t co2 = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read(); // CRC überspringen

        // 2. Temperatur (Umrechnen)
        uint16_t tempRaw = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read(); // CRC überspringen
        float temp = -45.0 + 175.0 * (float)tempRaw / 65535.0;

        // 3. Luftfeuchtigkeit (Umrechnen)
        uint16_t humRaw = (uint16_t)Wire.read() << 8 | Wire.read();
        Wire.read(); // CRC überspringen
        float hum = 100.0 * (float)humRaw / 65535.0;

        // CSV Ausgabe
        Serial.print(millis()); Serial.print(";");
        Serial.print(co2);      Serial.print(";");
        Serial.print(temp, 2);  Serial.print(";");
        Serial.println(hum, 2);
    }

    delay(5000); // Messintervall 5 Sek.
}