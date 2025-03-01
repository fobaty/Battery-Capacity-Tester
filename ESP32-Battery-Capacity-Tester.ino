#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

#define RELAY_PIN 20
#define BUZZER_PIN 19
#define VOLTAGE_PIN 1
#define R1 150000.0
#define R2 10000.0

const char* ssid = "BatteryTesterAP";
const char* password = "";

WebServer server(80);
float batteryVoltage = 0.0;
float loadResistance = 3.3;
bool testRunning = false;
unsigned long testStartTime;
unsigned long testDuration = 0;
unsigned long lastLogTime = 0;
unsigned long logInterval = 300000;
float dischargedAh = 0.0;
float voltageThreshold = 10.5;

void logTestData(unsigned long elapsedTime, float dischargeCurrent);

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        while (1);
    }

    WiFi.softAP(ssid, password);
    Serial.println("Access Point started!");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    
    server.on("/", handleRoot);
    server.on("/start", handleStartTest);
    server.on("/stop", handleStopTest);
    server.on("/set_threshold", handleSetThreshold);
    server.on("/set_resistance", handleSetResistance);
    server.begin();
    Serial.println("Web server started!");
}

void loop() {
    server.handleClient();
    
   int rawADC = analogRead(VOLTAGE_PIN);
   float voltageOut = (rawADC / 4095.0) * 3.3;  // Если 12-битный ADC и 3.3V питание
batteryVoltage = voltageOut * ((R1 + R2) / R2);

Serial.print("Raw ADC: ");
Serial.println(rawADC);
Serial.print("Voltage Out: ");
Serial.println(voltageOut);
Serial.print("Battery Voltage: ");
Serial.println(batteryVoltage);


    float dischargeCurrent = batteryVoltage / loadResistance;
    Serial.print("Battery Voltage: ");
    Serial.println(batteryVoltage);
    Serial.print("Discharge Current: ");
    Serial.println(dischargeCurrent);
    
    if (testRunning) {
        unsigned long elapsedTime = (millis() - testStartTime) / 1000;
        dischargedAh = (dischargeCurrent * elapsedTime) / 3600.0;
        
        if (batteryVoltage < voltageThreshold) {
            Serial.println("Voltage below threshold! Stopping test.");
            stopTest();
        }
        
        if (millis() - lastLogTime >= logInterval) {
            lastLogTime = millis();
            logTestData(elapsedTime, dischargeCurrent);
        }
    }
    delay(1000);
}

void handleRoot() {
    unsigned long elapsedTime = (testRunning) ? (millis() - testStartTime) / 1000 : testDuration;
    unsigned long minutes = elapsedTime / 60;
    unsigned long seconds = elapsedTime % 60;
    float dischargeCurrent = batteryVoltage / loadResistance;
    
    String html = "<html><body>";
    html += "<h1>Battery Tester v1</h1>";
    html += "<p>Battery Voltage: " + String(batteryVoltage) + "V</p>";
    html += "<p>Discharge Current: " + String(dischargeCurrent, 2) + "A</p>";
    html += "<p>Load Resistance: " + String(loadResistance, 2) + " Omhs</p>";
    html += "<p>Discharged Capacity: " + String(dischargedAh, 3) + " Ah</p>";
    
    if (!testRunning && testDuration > 0) {
        html += "<h2>Test Results</h2>";
        html += "<p><strong>Test Stopped!</strong></p>";
        html += "<p>Final Voltage: " + String(batteryVoltage) + "V</p>";
        html += "<p>Discharged Capacity: " + String(dischargedAh, 3) + " Ah</p>";
        html += "<p>Test Duration: " + String(minutes) + " min " + String(seconds) + " sec</p>";
    }
    
    html += "<p><a href='/start'>Start Test</a> | <a href='/stop'>Stop Test</a></p>";
    if (testRunning) {
        html += "<p>Test running for: " + String(minutes) + " min " + String(seconds) + " sec</p>";
    }
    
    html += "<hr>";
    
    html += "<p><form action='/set_threshold' method='POST'>";
    html += "Set Voltage Threshold: <input type='number' name='threshold' step='0.1' min='0' value='" + String(voltageThreshold) + "'> V<br>";
    html += "<input type='submit' value='Set Threshold'>";
    html += "</form></p>";
    
    html += "<p><form action='/set_resistance' method='POST'>";
    html += "Set Load Resistance: <input type='number' name='resistance' step='0.1' min='0' value='" + String(loadResistance) + "'> ohm<br>";
    html += "<input type='submit' value='Set Resistance'>";
    html += "</form></p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleStartTest() {
    testRunning = true;
    testStartTime = millis();
    dischargedAh = 0.0;
    testDuration = 0;
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Test started.");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleStopTest() {
    stopTest();
    server.sendHeader("Location", "/");
    server.send(303);
}

void stopTest() {
    testRunning = false;
    testDuration = (millis() - testStartTime) / 1000;
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Test stopped.");
}

void handleSetThreshold() {
    if (server.hasArg("threshold")) {
        voltageThreshold = server.arg("threshold").toFloat();
        Serial.print("Voltage threshold set to: ");
        Serial.println(voltageThreshold);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleSetResistance() {
    if (server.hasArg("resistance")) {
        loadResistance = server.arg("resistance").toFloat();
        Serial.print("Load resistance set to: ");
        Serial.println(loadResistance);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void logTestData(unsigned long elapsedTime, float dischargeCurrent) {
    File file = SPIFFS.open("/test.txt", FILE_APPEND);
    if (file) {
        file.printf("Time: %lu sec, Voltage: %.2fV, Discharged: %.3f Ah, Current: %.2f A\n", elapsedTime, batteryVoltage, dischargedAh, dischargeCurrent);
        file.close();
        Serial.println("Data logged.");
    }
}
