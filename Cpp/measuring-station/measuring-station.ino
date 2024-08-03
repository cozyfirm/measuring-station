// Example testing sketch for various DHT humidity/temperature sensors written by ladyada
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid = "Martin Routher King";
const char* password = "sifrajesifra";

//Your Domain name with URL path or IP address with path
String serverName = "https://weather-api.cozyfirm.com/api/weather/device/save";
String sensorReadings;

// Code from HTTP Request;
// 0000 represents success, otherwise error code
char responseCode[4];

// Time between two requests in miliseconds; Total 10 seconds
int delayTime = 10000;

#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  if(WiFi.status()== WL_CONNECTED){
      sensorReadings = httpPOSTRequest(serverName,t, h);
      Serial.println(sensorReadings);

      JSONVar myObject = JSON.parse(sensorReadings);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      strcpy(responseCode, myObject["code"]);

      Serial.print("Response Code: ");
      if(strcmp(responseCode, "0000") == 0){
        blinkLed(2);
      }else blinkLed(4);

      Serial.println(myObject["code"]);
    }
    else {
      Serial.println("WiFi Disconnected");
    }

  // Wait a few seconds between measurements.
  delay(delayTime);
}

String httpPOSTRequest(String serverName, int temperature, int humidity) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(serverName);
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // If you need an HTTP request with a content type: application/json, use the following:
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  int httpResponseCode = http.POST("{\"api_token\":\"6a3f778af7cdd7e3beee12884d6b8dabd33dad476dc8bb9e1f1fd9574dd895c0\",\"temperature\":\"" + String(temperature) + "\",\"humidity\":\"" + String(humidity) + "\",\"station\":\"2\"}");
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
    
  // Free resources
  http.end();

  return payload;
}

void blinkLed(int number){
  for(int i=0; i<number; i++){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(200);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(200);                      // wait for a second
  }
}
