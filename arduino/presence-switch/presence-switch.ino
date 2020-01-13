#include "config.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

String host = IFTT_ORIGIN;
String key = IFTT_KEY;
String eventOff = IFTT_EVENT_OFF;
String eventOn = IFTT_EVENT_ON;

byte read_index = 0;
unsigned int read_distance[BUFFER_READS];

byte latestPost = 0; // OFF

WiFiClient client;
HTTPClient http;

void setup() {
 
#ifdef DEBUG 
  Serial.begin(9600);
#endif

  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIGGER, LOW);

  for (byte i = 0; i < BUFFER_READS; i++) {
    read_distance[i] = NO_PRESENCE_DISTANCE * 2; // initila value
  }

#ifdef DEBUG 
  Serial.println();
  Serial.print("Conectando wifi: ");
  Serial.println(WIFI_SSID);
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG 
    Serial.print(".");
#endif
  }

#ifdef DEBUG 
  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
#endif
}

void postSwitch(String event) {
  if (WiFi.status()== WL_CONNECTED){ 
    http.begin(host + "/trigger/" + event + "/with/key/" + key);
    http.POST("");
    http.end();

#ifdef DEBUG 
    Serial.println("POSTED " + event);
#endif
  }
}

void loop() {
  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(2);
  
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);

  read_index = read_index + 1 > BUFFER_READS ? 0 : read_index + 1;
  const long duration = pulseIn(PIN_ECHO, HIGH);
  read_distance[read_index] = duration * 0.034/2;

#ifdef DEBUG 
  for (byte i = 0; i < BUFFER_READS; i++) {
    Serial.print(read_distance[i]);
    Serial.print(" | ");
  }
  Serial.println();
#endif

  byte no_presence_count = 0;
  
  for (byte i = 0; i < BUFFER_READS; i++) {
    if (read_distance[i] > NO_PRESENCE_DISTANCE) {
      no_presence_count++;
    }
  }
  
  if (no_presence_count >= NO_PRESENCE_READS) {
#ifdef DEBUG 
  Serial.println("POST OFF");
#endif

    if (latestPost == 1){
      postSwitch(eventOff);
      latestPost = 0;
    }
  }
  else {
#ifdef DEBUG 
  Serial.println("POST ON");
#endif

    if (latestPost == 0){
      postSwitch(eventOn);
      latestPost = 1;
    }
  }

  delay(READ_INTERVAL);
}
