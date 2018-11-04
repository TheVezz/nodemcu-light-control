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
#include <Chrono.h>
#define relayPin D0
#define buttonPin D1
int relayState = LOW;
Chrono myChrono;
int previousButtonState;
int LastRead = LOW;
bool connLost = false;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "you-ssid"
#define WLAN_PASS       "you-psw"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                 // use 8883 for SSL, 1883 default
#define AIO_USERNAME    "you-username"
#define AIO_KEY         "tou-aio-key"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/sample");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
//bool MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  previousButtonState = digitalRead(buttonPin);
  digitalWrite(relayPin, relayState);

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

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

//uint32_t x=0;

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
bool MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return true;
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
      return false;
    }
  }
  Serial.println("MQTT Connected!");
  return true;
}


void loop() {
  // Verifico se non ho perso la connessione
  if (!connLost) {
    // Ok, verifica la connessione o prova a (ri)connettersi
    if (!MQTT_connect()) {
      // Connessione/riconnessione fallita! Lascio perdere
      connLost = true;
    } else {
      // La connessione è ok
      // Google assistant
      Adafruit_MQTT_Subscribe *subscription;
      while ((subscription = mqtt.readSubscription(10))) {
        if (subscription == &onoffbutton) {
          Serial.println("******************");
          Serial.print(F("Got: "));
          Serial.println((char *)onoffbutton.lastread);
          relayState = atoi((char *)onoffbutton.lastread);
          digitalWrite(relayPin, relayState);
        }
      }

    }
  }

  //la connessione non è ok
  //button
  int newButtonState = digitalRead(buttonPin);
  if ( previousButtonState != newButtonState ) {
    previousButtonState = newButtonState;

    if ( newButtonState == LOW && myChrono.hasPassed(50) ) {
      Serial.println("******************");
      Serial.print("ms ");
      Serial.print( myChrono.elapsed() );
      Serial.println();
      relayState = !relayState;
      digitalWrite(relayPin, relayState);
      Serial.print(F("Set: "));
      Serial.println(relayState);

    }

    myChrono.restart();
  }


  if (connLost == true && myChrono.hasPassed(200000)) {
    Serial.println("******************");
    Serial.print("ms ");
    Serial.print( myChrono.elapsed() );
    Serial.println();
    connLost = false;
    MQTT_connect();
    myChrono.start();
    // DO SOMETHING EVERY 200 MS.

  }
}
