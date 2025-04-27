#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  
const byte address[6] = "00001";

const byte secretKey = 0x5A;  // Any 8-bit secret key

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
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
}

void loop() {
    data.curTime = millis() / 1000;
    data.temp = 34.87;
    data.humidity = 61.93;
    data.correctedPPM = 25.37;
    data.voltage = 234.56;
    data.current = 13.98;

    byte* ptr = (byte*)&data;
    for (int i = 0; i < sizeof(data); i++) {
        ptr[i] ^= secretKey; // XOR encryption
    }

    radio.write(&data, sizeof(data));
    Serial.println("Data sent.");
    delay(3000);
}
