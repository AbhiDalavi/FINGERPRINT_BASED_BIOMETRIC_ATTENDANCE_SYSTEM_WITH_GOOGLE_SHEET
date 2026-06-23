#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <Wire.h>

#define RELAY_PIN D2  // Define relay pin

struct User {
    String name;
    int id;
    bool hasScanned; // Track if the user has scanned for IN Time
};

User users[] = { 
    {"Abhi", 1, false}, 
    {"Aditya", 2, false}, 
    {"Darshan", 3, false}, 
    {"Rushi", 4, false}, 
    {"Virendra", 6, false}, 
    {"Anup", 7, false}, 
    {"Pranav", 10, false}, 
    
};

bool relayState[10] = {0}; // Track relay state for each user

SoftwareSerial mySerial(D4, D3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const char* ssid = "Abhi";
const char* password = "abhi1289";
const char *GScriptId = "AKfycbw2FYk_c3otob45jzSxJ6SjWBN2owwOhODKsOL_TDu-LLZ_FphVZDeiuhbsYRHwVUDD4w";
const char* host = "script.google.com";
const int httpsPort = 443;
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
    Serial.begin(9600);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    
    finger.begin(57600);
    if (!finger.verifyPassword()) {
        Serial.println("Fingerprint sensor not found!");
        while (1) delay(1);
    }
    Serial.println("Fingerprint sensor found!");
}

void loop() {
    displayMessage("Waiting...");
    int fingerID = getFingerprintID();
    if (fingerID > 0) {
        processFingerprint(fingerID);
        displayMessage("Success");
    }
    delay(500);
}

void processFingerprint(int id) {
    for (int i = 0; i < 11; i++) {
        if (users[i].id == id) {
            if (!users[i].hasScanned) {
                // First scan: IN Time
                updatesheet(users[i].name, String(users[i].id), "IN");
                users[i].hasScanned = true;
            } else {
                // Second scan: OUT Time
                updatesheet(users[i].name, String(users[i].id), "OUT");
                users[i].hasScanned = false; // Reset for the next scan
            }
            relayState[i] = !relayState[i];
            digitalWrite(RELAY_PIN, relayState[i] ? HIGH : LOW);
            return;
        }
    }
    Serial.println("User not recognized.");
}

void updatesheet(String user, String id, String scanType) {
    if (!client) {
        client = new HTTPSRedirect(httpsPort);
        client->setInsecure();
    }
    if (!client->connected()) {
        client->connect(host, httpsPort);
    }
    
    String payload = "{\"command\": \"update_row\", \"sheet_name\": \"Sheet1\", \"values\": \"" + user + "," + id + "," + scanType + "\"}";
    Serial.println("Publishing data...");
    Serial.println(payload);
    if (!client->POST(url, host, payload)) {
        Serial.println("Error while connecting to Google Sheets");
    }
    delay(5000);
}

void displayMessage(String msg) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print(msg);
    display.display();
}

int getFingerprintID() {
    if (finger.getImage() != FINGERPRINT_OK) return -1;
    if (finger.image2Tz() != FINGERPRINT_OK) return -1;
    if (finger.fingerSearch() != FINGERPRINT_OK) return -1;
    Serial.print("Found ID #"); Serial.println(finger.fingerID);
    return finger.fingerID;
}
