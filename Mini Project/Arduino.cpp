#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <TinyGPS++.h>
#include <WiFiEsp.h>

// GSM and GPS pins
SoftwareSerial gpsSerial(4, 5);   // RX, TX for GPS
SoftwareSerial gsmSerial(7, 8);   // RX, TX for GSM

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
TinyGPSPlus gps;
char server[] = "your-backend-server.com";  // Backend server for accident data

void setup() {
    Serial.begin(9600);

    // GSM and GPS
    gsmSerial.begin(9600);
    gpsSerial.begin(9600);

    // Accelerometer
    if(!accel.begin()) {
        Serial.println("No accelerometer detected");
        while(1);
    }
    accel.setRange(ADXL345_RANGE_16_G);
}

void loop() {
    sensors_event_t event;
    accel.getEvent(&event);

    if (detectAccident(event)) {
        String gpsData = getGPSLocation();
        sendSMS(gpsData);            // Send SMS and make a call
        dialEmergency();
        sendAccidentData(gpsData);   // Send data to backend
    }

    delay(5000);  // Check every 5 seconds
}

bool detectAccident(sensors_event_t event) {
    // Detect sudden impact or movement change
    if (abs(event.acceleration.x) > 5 || abs(event.acceleration.y) > 5 || abs(event.acceleration.z) > 5) {
        return true;
    }
    return false;
}

String getGPSLocation() {
    String gpsData = "";
    while (gpsSerial.available() > 0) {
        if (gps.encode(gpsSerial.read())) {
            gpsData = "Lat: " + String(gps.location.lat(), 6) + ", Lon: " + String(gps.location.lng(), 6);
        }
    }
    return gpsData;
}

void sendSMS(String gpsData) {
    gsmSerial.println("AT+CMGF=1");  // Set SMS mode
    delay(100);
    gsmSerial.println("AT+CMGS=\"+911234567890\"");  // Emergency contact
    delay(100);
    gsmSerial.print("Accident detected! Location: ");
    gsmSerial.print(gpsData);
    gsmSerial.write(26);  // CTRL+Z to send
    delay(1000);
}

void dialEmergency() {
    gsmSerial.println("ATD911;");  // Dial emergency number
    delay(30000);  // Call duration of 30 seconds
    gsmSerial.println("ATH");  // Hang up
}

void sendAccidentData(String gpsData) {
    // Code to send accident data to a backend server via WiFi/GSM module
    gsmSerial.println("AT+HTTPINIT");
    delay(100);
    gsmSerial.println("AT+HTTPPARA=\"URL\",\"http://" + String(server) + "/accident\"");
    delay(100);
    gsmSerial.println("AT+HTTPDATA=100,10000");
    delay(100);
    gsmSerial.println("POST data here: " + gpsData);  // Replace with actual data
    delay(100);
    gsmSerial.println("AT+HTTPACTION=1");  // Send POST request
    delay(100);
    gsmSerial.println("AT+HTTPTERM");  // Close HTTP connection
}
