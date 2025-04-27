#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define RF_CS 10
#define RF_CE 9 

RF24 radio(RF_CE, RF_CS);
const byte address[6] = "00001";

const byte secretKey = 0x5A;

struct SensorData {
    unsigned long curTime;   // 4 bytes
    float temp;              // 4 bytes
    float humidity;          // 4 bytes
    float correctedPPM;      // 4 bytes
    float voltage;           // 4 bytes
    float current;           // 4 bytes
};

SensorData data;    

void setup() {
    Serial.begin(9600);

    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
}

void loop() {
    if (radio.available()) {
        radio.read(&data, sizeof(data));

        byte* ptr = (byte*)&data;
        for (int i = 0; i < sizeof(data); i++) {
            ptr[i] ^= secretKey; // XOR decryption
        }

        Serial.print(data.curTime);
        Serial.print(",");
        Serial.print(data.temp, 2);
        Serial.print(",");
        Serial.print(data.humidity, 2);
        Serial.print(",");
        Serial.print(data.correctedPPM, 2);
        Serial.print(",");
        Serial.print(data.voltage, 2);
        Serial.print(",");
        Serial.println(data.current, 2);
    }
}
