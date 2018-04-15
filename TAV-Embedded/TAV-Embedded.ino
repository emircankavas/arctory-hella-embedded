#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Redmi2"
#define WLAN_PASS       "45448585788"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "emircankavas"
#define AIO_KEY         "90cae602d5be4f3d95238982966307d4"

/************************* Hella Device Values *******************************/

char *DEVICE_ID   =       "5ad255f6a09ef90014a77157";
char *TENANT_ID   =       "5ad20dcca7b49509952d9a9f";
char *DEVICE_CODE =       "3";

/************ Global State (you don't need to change this!) ******************/

boolean deviceState = true;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish HELLA_PROBLEM_CREATED = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/hella-problem-created");
Adafruit_MQTT_Subscribe HELLA_PROBLEM_SOLVED = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/hella-problem-solved");

/*************************** Sketch Code ************************************/

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode( LED_BUILTIN, OUTPUT);
  pinMode( D1, OUTPUT);

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

  mqtt.subscribe(&HELLA_PROBLEM_SOLVED);
}

String payload = "{\"tenantId\": \"5ad20dcca7b49509952d9a9f\",\"deviceId\": \"5ad255f6a09ef90014a77157\",\"problemCode\": \"3\"}";
char * cPayload = &payload[1u];

void loop() {
  int IRSensor = analogRead(A0);

  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &HELLA_PROBLEM_SOLVED) {
      Serial.print(F("Got: "));
      Serial.println((char *)HELLA_PROBLEM_SOLVED.lastread);
      if ( !strcmp( (char *)HELLA_PROBLEM_SOLVED.lastread ,DEVICE_ID ))
      digitalWrite( LED_BUILTIN, HIGH );
      delay(250);
      digitalWrite( LED_BUILTIN, LOW );
      delay(250);
          deviceState = true;
    }
  }

  if ( IRSensor <= 900 && deviceState) {
     // digitalWrite(LED_BUILTIN, LOW);
      if (! HELLA_PROBLEM_CREATED.publish(cPayload)) {
          Serial.println("Failed");
      } else {
      Serial.println("OK!");
      deviceState = false;
      digitalWrite( LED_BUILTIN, HIGH );
      delay(250);
      digitalWrite( LED_BUILTIN, LOW );
      delay(250);
      }
      //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
}

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
       delay(10000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
