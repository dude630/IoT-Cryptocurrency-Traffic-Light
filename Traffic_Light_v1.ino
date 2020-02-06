#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

//https fingerprint for CoinGecko, expires 4/18/2020 and will need to be updated.
const uint8_t fingerprint[20] = {0x37, 0xf0, 0x60, 0x3a, 0x1b, 0xe1, 0x3d, 0x5f, 0x0d, 0x93, 0x44, 0x62, 0x19, 0xb4, 0x69, 0xcd, 0x6d, 0x28, 0x8f, 0x68};

ESP8266WiFiMulti WiFiMulti;

const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 60; //dynamic buffer for ArduinoJson library
DynamicJsonBuffer jsonBuffer(capacity);

//traffic light specific variables
double btcVal;
double prevBtcVal = 0;
double dayChangeVal;
int green = 13;
int yellow = 12;
int red = 14;

void setup() {
  Serial.begin(115200);

  //give ESP8266 time to start up
  delay(4000);

  WiFi.mode(WIFI_STA);
  //
  WiFiMulti.addAP("SSID", "PASSWORD");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
}

void loop() {
  grabData();
  Serial.println(btcVal);
  Serial.println(dayChangeVal);
  if (btcVal != prevBtcVal) {
    if (btcVal > prevBtcVal) {
      Serial.println("Bitcoin value has gone up!");
      wentUp();
      prevBtcVal = btcVal;
    }
    else if (btcVal < prevBtcVal) {
      Serial.println("Bitcoin value has gone down!");
      wentDown();
      prevBtcVal = btcVal;
    }
  }
  else if (btcVal == prevBtcVal) {
    Serial.println("Bitcoin value has stayed the same.");
    stayedSame();
  }
  if (dayChangeVal >= 1) {
    Serial.println("Bitcoin is trending upwards!");
    trendingUp();
  }
  else if (dayChangeVal <= -1) {
    Serial.println("Bitcoin is trending downwards!");
    trendingDown();
  }
  else if (dayChangeVal < 1 && dayChangeVal > -1) {
    Serial.println("Bitcoin price is staying about the same.");
    trendingMid();
  }
  delay(60000);
}

void grabData() {
  if (WiFiMulti.run() == WL_CONNECTED) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint); //Sets HTTPS fingerprint

    HTTPClient https;

    if (https.begin(*client, "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&include_24hr_change=true")) {
      int httpCode = https.GET(); 
      
      if (httpCode > 0) { //checks if webpage was successfully grabbed
        String payload = https.getString();
        JsonObject& root = jsonBuffer.parseObject(payload);
        btcVal = root["bitcoin"]["usd"];
        dayChangeVal = root["bitcoin"]["usd_24h_change"];
      }
    }
  }
}

void wentUp() {
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);
  for (int i = 2; i > 0; i--) {
    digitalWrite(green, HIGH);
    delay(300);
    digitalWrite(green, LOW);
    delay(300);
  }
}

void wentDown() {
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);
  for (int i = 2; i > 0; i--) {
    digitalWrite(red, HIGH);
    delay(300);
    digitalWrite(red, LOW);
    delay(300);
  }
}

void stayedSame() {
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);
  for (int i = 2; i > 0; i--) {
    digitalWrite(yellow, HIGH);
    delay(300);
    digitalWrite(yellow, LOW);
    delay(300);
  }
}

void trendingUp() {
  digitalWrite(green, HIGH);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);
}

void trendingDown() {
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, HIGH);
}

void trendingMid() {
  digitalWrite(green, LOW);
  digitalWrite(yellow, HIGH);
  digitalWrite(red, LOW);
}
