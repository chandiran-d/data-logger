#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Setup nRF24L01
RF24 radio(9, 10); // CE, CSN pins
const byte address[6] = "00001"; // Same address as transmitter

// Secret key for XOR decryption (must match transmitter)
const byte secretKey = 0x5A;

// Define the structure (must match exactly with transmitter)
struct SensorData {
    unsigned long curTime;
    float temp;
    float humidity;
    float correctedPPM;
    float voltage;
    float current;
};

SensorData data;    

void setup() {
    Serial.begin(9600);

    // Initialize RF24
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
}

void loop() {
    if (radio.available()) {
        radio.read(&data, sizeof(data));

        // Decrypt the incoming data   
        byte* ptr = (byte*)&data;
        for (int i = 0; i < sizeof(data); i++) {
            ptr[i] ^= secretKey; // XOR decryption
        }

        // Print the decrypted data 
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
