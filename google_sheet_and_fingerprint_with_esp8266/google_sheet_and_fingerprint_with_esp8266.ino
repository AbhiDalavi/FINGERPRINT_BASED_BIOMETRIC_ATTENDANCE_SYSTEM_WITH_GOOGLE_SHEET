
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <Wire.h>
#define relay D2  // Define relay pin
int u=0;
int v=0;

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(D4, D3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1
 
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Enter network credentials:
const char* ssid     = "Abhi";
const char* password = "abhi1289";

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbz5LhtxTRv3ouwzmC_b0lnjZMOpQ1J0izzTsrMTkFV2ze_qrZOh_IoQ_TR25p7NQprj";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Declare variables that will be published to Google Sheets
String user = "";
String id = "";
String enter ="";

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {

  Serial.begin(9600);        

  Serial.println('\n');

  Serial.println("System initialized");



  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  delay(10);


while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");
 
  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
 
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
 
  finger.getTemplateCount();
 
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
  
}


void loop() {

  // create some fake data to publish
display.clearDisplay();
     display.setTextSize(2);
  display.setCursor(0, 10);
 display.print("Waiting..."); 
  display.display();

getFingerprintID();
  delay(50);            //don't ned to run this at full speed.
display.clearDisplay();
     display.setTextSize(2);
  display.setCursor(0, 10);
 display.print("Success"); 
  display.display();
  
  }
  

void updatesheet(String user, String id, String enter)

{


  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + user + "," + id + "," + enter + "\"}";
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  delay(5000);
  }



uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_BLUE);
      finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_RED);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
 
  // OK success!
 
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
 
  // OK converted!
  p = finger.fingerSearch();
  Serial.println("finger print id");
  Serial.println(p);
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_PURPLE, 10);
    delay(1000);
    if(finger.fingerID==1)
    {
  if(u==0)
    {
      digitalWrite(relay,HIGH);
       user="Abhi";
      id="1";
      enter="in";
       updatesheet(user,id, enter);
      
      u=1;
    }
    else if(u==1)
    {
      digitalWrite(relay,LOW);
      enter="";
      user="Abhi";
      id="1";
      enter="out";
             updatesheet(user,id, enter);
      u=0;
    }
}
   if(finger.fingerID==3) 
   {
    if(v==0)
    {
      digitalWrite(relay,HIGH);
      enter="";
      user="Aditya";
      id="2";
      enter="in";
             updatesheet(user,id, enter);
      
      v=1;
    }

    
     else if(v==1)
    {
      digitalWrite(relay,HIGH);
      enter="";
      user="Aditya";
      id="2";
      enter="out";
             updatesheet(user,id, enter);
      
      v=0;
    }
  }
  if(finger.fingerID==5)
    {
  if(u==0)
    {
      digitalWrite(relay,HIGH);
       user="Rushi";
      id="3";
      enter="in";
       updatesheet(user,id, enter);
      
      u=1;
    }
    else if(u==1)
    {
      digitalWrite(relay,LOW);
      enter="";
      user="Rushi";
      id="3";
      enter="out";
             updatesheet(user,id, enter);
      u=0;
    }
}
if(finger.fingerID==6)
    {
  if(u==0)
    {
      digitalWrite(relay,HIGH);
       user="Darshan";
      id="4";
      enter="in";
       updatesheet(user,id, enter);
      
      u=1;
    }
    else if(u==1)
    {
      digitalWrite(relay,LOW);
      enter="";
      user="Darshan";
      id="4";
      enter="out";
             updatesheet(user,id, enter);
      u=0;
    }
}
  }
  
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
  delay(1000);
  Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
 
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
 
  return finger.fingerID;
}
 
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;
 
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;
 
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
 
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
