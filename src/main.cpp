#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include <connect.h>
#include <staticserve.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     14  //the D5 on D1Mini
#define LED_COUNT  120
#define BRIGHTNESS 50
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
#define DEBUG

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer httpServer(80);
WebSocketsServer webSocket(81);

void setupStripe(void) {
        strip.begin();
        strip.show();
        strip.setBrightness(BRIGHTNESS);
}

void handleNotFound() {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += httpServer.uri();
        message += "\nMethod: ";
        message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += httpServer.args();
        message += "\n";

        for (uint8_t i = 0; i < httpServer.args(); i++) {
                message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
        }

        httpServer.send(404, "text/plain", message);
}

bool loadFromSPIFFS(String path, String dataType) {
        Serial.print("Requested page -> ");
        Serial.println(path);
        if (SPIFFS.exists(path)) {
                File dataFile = SPIFFS.open(path, "r");
                if (!dataFile) {
                        handleNotFound();
                        return false;
                }

                if (httpServer.streamFile(dataFile, dataType) != dataFile.size()) {
                        Serial.println("Sent less data than expected!");
                }else{
                        Serial.println("Page served!");
                }

                dataFile.close();
        }else{
                handleNotFound();
                return false;
        }
        return true;
}

String getContentType(String filename){
        if(filename.endsWith(F(".htm"))) return F("text/html");
        else if(filename.endsWith(F(".html"))) return F("text/html");
        else if(filename.endsWith(F(".css"))) return F("text/css");
        else if(filename.endsWith(F(".js"))) return F("application/javascript");
        else if(filename.endsWith(F(".json"))) return F("application/json");
        else if(filename.endsWith(F(".png"))) return F("image/png");
        else if(filename.endsWith(F(".gif"))) return F("image/gif");
        else if(filename.endsWith(F(".jpg"))) return F("image/jpeg");
        else if(filename.endsWith(F(".jpeg"))) return F("image/jpeg");
        else if(filename.endsWith(F(".ico"))) return F("image/x-icon");
        else if(filename.endsWith(F(".xml"))) return F("text/xml");
        else if(filename.endsWith(F(".pdf"))) return F("application/x-pdf");
        else if(filename.endsWith(F(".zip"))) return F("application/x-zip");
        else if(filename.endsWith(F(".gz"))) return F("application/x-gzip");
        else if(filename.endsWith(F(".ttf"))) return F("font/ttf");
        return F("text/plain");
}

bool handleFileRead(String path){
        Serial.print(F("handleFileRead: "));
        Serial.println(path);

        if(path.endsWith("/")) path += F("index.html");
        String contentType = getContentType(path);
        String pathWithGz = path + F(".gz");
        if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
                if(SPIFFS.exists(pathWithGz))
                        path += F(".gz");
                fs::File file = SPIFFS.open(path, "r");
                size_t sent = httpServer.streamFile(file, contentType);
                file.close();
                Serial.println(String(F("\tSent file: ")) + path + String(F(" of size ")) + sent);
                return true;
        }
        Serial.println(String(F("\tFile Not Found: ")) + path);
        return false;
}

void serverRouting() {
        httpServer.onNotFound([]() {
                Serial.println(F("On not found"));
                if (!handleFileRead(httpServer.uri())) {
                        handleNotFound();
                }
        });
        Serial.println(F("Set cache!"));
        httpServer.serveStatic("/configuration.json", SPIFFS, "/configuration.json","no-cache, no-store, must-revalidate");
        httpServer.serveStatic("/", SPIFFS, "/","max-age=31536000");
}

int getNum(char ch){
        int num=0;
        if(ch>='0' && ch<='9')
        {
                num=ch-0x30;
        }
        else
        {
                switch(ch)
                {
                case 'A': case 'a': num=10; break;
                case 'B': case 'b': num=11; break;
                case 'C': case 'c': num=12; break;
                case 'D': case 'd': num=13; break;
                case 'E': case 'e': num=14; break;
                case 'F': case 'f': num=15; break;
                default: num=0;
                }
        }
        return num;
}

unsigned int hex2int(unsigned char hex[]){
        unsigned int x=0;
        x=(getNum(hex[0]))*16+(getNum(hex[1]));
        return x;
}

void setStripeColor(int r, int g, int b){
        for (size_t Pixel = 0; Pixel < LED_COUNT; Pixel++) {
                strip.setPixelColor(Pixel, r,g,b);
        }
        strip.show();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
        switch (type) {

        case WStype_DISCONNECTED: {
          #ifdef DEBUG
                Serial.printf("[%u] Disconnected!\n", num);
          #endif
                break;
        }

        case WStype_CONNECTED: {
                IPAddress ip = webSocket.remoteIP(num);
                webSocket.sendTXT(num, "HUIHIH");
          #ifdef DEBUG
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
          #endif
                break;
        }

        case WStype_TEXT: {
          #ifdef DEBUG
                Serial.printf("[%u] get Text: %s\n", num, payload);
                int z = (int)(payload[0]);
                Serial.printf("%d\n", z);
          #endif

                switch ((int)(payload[0])) {
                case 67: {
                        break;
                }
                case 35: {
                        unsigned char r[]={payload[1],payload[2]};
                        unsigned char g[]={payload[3],payload[4]};
                        unsigned char b[]={payload[5],payload[6]};
                        setStripeColor(hex2int(r),hex2int(g),hex2int(b));

                        #ifdef DEBUG
                        Serial.printf("get R value: %i get G value: %i get B value: %i\n", hex2int(r), hex2int(g), hex2int(b));
                        #endif

                        break;
                }

                }
        }

        // For everything else: do nothing
        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        default:
                break;

        }
}

void startWebSocket() {
        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
        Serial.println("WebSocket server started.");
}

//Command for serial Monitor: pio device monitor -b 115200 -f colorize -f send_on_enter -f time
//Data upload: pio run -t uploadfs

void setup() {
        delay(1000);
        Serial.begin(115200);
        pinMode(LED_BUILTIN, OUTPUT);
        delay(100);
        connectToNetwork();

        Serial.print(F("Inizializing FS..."));
        if (SPIFFS.begin()) {
                Serial.println(F("done."));
        }else{
                Serial.println(F("fail."));
        }

        serverRouting();
        httpServer.begin();
        startWebSocket();
        setupStripe();
}


void loop() {
        if (WiFi.status() != WL_CONNECTED) {
                delay(10);
                connectToNetwork();
        }
        webSocket.loop();
        httpServer.handleClient();
}
