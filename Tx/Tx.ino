#include <SPI.h>
#include <DHT.h>
#include <MQ135.h>
#include <nRF24L01.h>
#include <RF24.h>

#define DHTPIN 2        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22 (AM2302), AM2321

#define PIN_MQ135 A0    // MQ135 Analog Input Pin

const int VoltSensorPin = A2; // Analog pin connected to ZMPT101B OUT

const int CurSensorPin = A4;   // ACS712 output connected to A0

// Initialize DHT sensor 
DHT dht(DHTPIN, DHTTYPE);

// Initialize MQ135 sensor 
MQ135 mq135_sensor(PIN_MQ135);

// Read temperature as Celsius
float t = 0; 

// Read humidity 
float h = 0;

// Voltage 
float vMax = 0;
float vRMS = 0;
float voltage = 0;
float calib = 1.15;

// Current 
const float sensitivity = 0.185; // 185mV/A for ACS712-5A module
const float offset = 2.6;   // Offset voltage at 0A (approximately 2.5V)

const unsigned long writeInterval = 3000; // Log every 3 second 

unsigned long prevTime = 0;
unsigned long curTime = 0;

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

boolean serialPrint = true; // for debugging 

void setup() {
    Serial.begin(9600);
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
}

void loop() {
  curTime = millis();

  if (curTime - prevTime >= writeInterval) {
    prevTime = curTime;

    data.curTime = prevTime;

    // Reading temperature or humidity takes about 250 milliseconds
    t = dht.readTemperature();
    h = dht.readHumidity();

    // Check if the reading failed
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor! Retrying...");
      return;      // Skip to next loop iteration
    }
    
    data.temp = t; 
    data.humidity = h; 

    float rzero = mq135_sensor.getRZero();
    float correctedRZero = mq135_sensor.getCorrectedRZero(t, h);
    float resistance = mq135_sensor.getResistance();
    float ppm = mq135_sensor.getPPM();
    float correctedPPM = mq135_sensor.getCorrectedPPM(t, h);

    vMax = 0; // Reset max voltage reading

    // Read multiple values to find peak voltage
    for (int i = 0; i < 1000; i++) {
        int sensorValue = analogRead(VoltSensorPin);
        float voltage = sensorValue * (5.0 / 1023.0); // Convert to voltage
        if (voltage > vMax) {
            vMax = voltage;
        }
    }

    // Calculate RMS voltage assuming sinusoidal wave
    vRMS = vMax * 0.707; 
    voltage = vRMS * (230.0 / 2.5) * calib; // Scale factor (calibration required)

    data.voltage = voltage; 

    int CurSensorValue = analogRead(CurSensorPin);
    float volt = (CurSensorValue / 1023.0) * 5.0; // Convert to voltage
    float current = ((volt - offset) / sensitivity) * 1000; // Calculate current in mA 

    data.current = current; 

    if(serialPrint) {
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

    byte* ptr = (byte*)&data;
    for (int i = 0; i < sizeof(data); i++) {
        ptr[i] ^= secretKey; // XOR encryption
    }

    radio.write(&data, sizeof(data));

    Serial.println("Data sent.");
  }
}
