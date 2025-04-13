#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <MQ135.h>

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

const unsigned long writeInterval = 2000; // Log every 2 second 
const unsigned long logDuration = 60000;  // Stop logging after 60 seconds (1 minute)

unsigned long prevTime = 0;
unsigned long curTime = 0;

boolean serialPrint = true;
const char *logFile = "datalog.csv";

const int chipSelect = 10;
File myFile;

void writeLog(unsigned long, float, float, float, float, float);

void setup() {
  Serial.begin(9600);

  // Wait for Serial Monitor to connect. Needed for native USB port boards only. 
  while (!Serial);

  dht.begin();

  Serial.print("Initializing SD card... ");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");

    while (true); // Halt the system if SD card fails 
  }

  Serial.println("Initialization done.");

  SD.remove(logFile); // Delete old log file 

  myFile = SD.open(logFile, FILE_WRITE);

  if (!myFile) {
    Serial.println("Log file missing");

    while(1); // Halt if file cannot be created 
  }

  myFile.println("Time (s),Temp (°C),Humidity (%),CO2 (PPM),Volt (V),Current (mA)"); // CSV Header

  myFile.close();
}

void loop() {
  curTime = millis();

  // Stop logging after 1 minute
  if (curTime >= logDuration) {
    Serial.println("Logging complete. Stopping program.");

    while (true);  // Halt execution
  }

  if (curTime - prevTime >= writeInterval) {
    prevTime = curTime;

    // Reading temperature or humidity takes about 250 milliseconds
    t = dht.readTemperature();
    h = dht.readHumidity();

    // Check if the reading failed
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor! Retrying...");
      return;      // Skip to next loop iteration
    }

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

    int CurSensorValue = analogRead(CurSensorPin);
    float volt = (CurSensorValue / 1023.0) * 5.0; // Convert to voltage
    float current = ((volt - offset) / sensitivity) * 1000; // Calculate current in mA 

    writeLog(curTime/1000, t, h, correctedPPM, voltage, current);
  }
}

void writeLog(unsigned long time, float temp, float hum, float conc, float volt, float cur) {
  myFile = SD.open(logFile, FILE_WRITE);

  if (myFile) {
    myFile.print(time);
    myFile.print(",");
    myFile.print(temp, 2);
    myFile.print(",");
    myFile.print(hum, 2);
    myFile.print(",");
    myFile.print(conc, 2);
    myFile.print(",");
    myFile.print(volt, 2);
    myFile.print(",");
    myFile.println(cur, 2);
    myFile.close();

    if(serialPrint) {
      Serial.print("Time: ");
      Serial.print(time);
      Serial.print("s\tTemperature: ");
      Serial.print(temp);
      Serial.print("°C\tHumidity: ");
      Serial.print(hum);
      Serial.print("%\tCO2: ");
      Serial.print(conc);
      Serial.print(" PPM\tVoltage: ");
      Serial.print(volt);
      Serial.print("V\tCurrent: ");
      Serial.print(cur);
      Serial.println("mA");
    }
  }

  else {
    Serial.println("Error opening log file!");
  }
}
