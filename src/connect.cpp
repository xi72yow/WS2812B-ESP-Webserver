#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "connect.h"
#include ".secret.h" // delete this line!

//!! Here you must declare your WIFI Infos
// const char* ssid = "SSID";
// const char* password = "password";

void connectToNetwork() {
        byte i = 0;
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
#ifdef NO_SLEEP
                pinMode(LED_BUILTIN, OUTPUT);
                digitalWrite(LED_BUILTIN, 0);
#endif
                delay(500);
                digitalWrite(LED_BUILTIN, 1);
                delay(500);
                i++;
                if (i > 9) {
                        ESP.restart();
                }
        }
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
}
