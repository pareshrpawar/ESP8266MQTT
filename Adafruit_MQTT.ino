/***************************************************
  Adafruit MQTT Library ESP8266 Example
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino
  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// the feed turns this RELAY on/off
#define RELAY1 D3  
#define RELAY2 D4
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "your ssid"
#define WLAN_PASS       "your password"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883                   // 1883 for http 8883 for https
#define AIO_USERNAME    "your user name"
#define AIO_KEY         "your secret key"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;  // Must set AIO_SERVERPORT to 1883
// or... use WiFiFlientSecure for SSL
WiFiClientSecure client;  // Must set AIO_SERVERPORT to 8883

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
// you can add more feeds here 
Adafruit_MQTT_Subscribe light1button = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/light1");
Adafruit_MQTT_Subscribe light2button = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/light2");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for your feeds.
  mqtt.subscribe(&light1button);
  mqtt.subscribe(&light2button);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  
  // if you added extra feeds you need to one more condition in the loop

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check which button feed
    if (subscription == &light1button) {
      Serial.print(F("On-Off button: "));
      Serial.println((char *)light1button.lastread);
      
      if (strcmp((char *)light1button.lastread, "0") == 0) {
        digitalWrite(RELAY1, LOW); 
      }
      if (strcmp((char *)light1button.lastread, "1") == 0) {
        digitalWrite(RELAY1, HIGH); 
      }
    }
     if (subscription == &light2button) {
      Serial.print(F("light2 button: "));
      Serial.println((char *)light2button.lastread);
      
      if (strcmp((char *)light2button.lastread, "0") == 0) {
        digitalWrite(RELAY2, LOW); 
      }
      if (strcmp((char *)light2button.lastread, "1") == 0) {
        digitalWrite(RELAY2, HIGH); 
      }
    }    
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
