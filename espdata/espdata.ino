
#include <ESP8266WiFi.h>
#include <MicroGear.h>

const char* ssid     = "Witawatd";
const char* password = "20102553";

#define APPID   "WorkShopLED"
#define KEY     "tjVAVjlbJHII6nq"
#define SECRET  "MgBnXJHN74Lm6ISZ3m3oogCSO"
#include <ESP8266WiFiMulti.h>
#define ALIAS   "smartlock"
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>
#include <Arduino.h>

WiFiClient client;

int timer = 0;
char str[32];

MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message -->");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
}

void setup() {


  microgear.on(MESSAGE, onMsghandler);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);
}

void loop() {


  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    String url = "http://iot.rmu.ac.th/iot/Smartlock/NEtpie.php";
    Serial.println(url);
    http.begin(url); //HTTP

    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);

        if (microgear.connected()) {
          Serial.println("connected");
          microgear.loop();

          if (timer >= 1000) {

            //String a = "dfsadfsa,nkjdsnfids";

            Serial.print("Sending -->");


            //String data = String(payload)+","+String(stat);
            //String Status = payload.substring(83);
            Serial.println(payload);
            microgear.publish("/door", String(payload));
            status();
            timer = 0;
          }
          else timer += 100;
        }
        else {
          Serial.println("connection lost, reconnect...");
          if (timer >= 5000) {
            microgear.connect(APPID);
            timer = 0;
          }
          else timer += 100;
        }
        delay(100);
      }

    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
void status() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http2;

    String url2 = "http://iot.rmu.ac.th/iot/Smartlock/status.php";
    //cam.printDebug(url2);
    http2.begin(url2); //HTTP

    int httpCode2 = http2.GET();
    if (httpCode2 > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode2);
      if (httpCode2 == HTTP_CODE_OK) {
        String payload2 = http2.getString();
        String iddoor = payload2.substring(0, 1);
        String status = payload2.substring(1);
        //cam.printDebug(payload);
        Serial.println(iddoor);
        Serial.println(status);
        if (status == "Unlock") {

          microgear.writeFeed("smartlock", "doorid1:1", "cqyqmXB6TV4z3QD8z855gZlQFivPyeQf");
          delay(5000);
          microgear.writeFeed("smartlock", "doorid1:0", "cqyqmXB6TV4z3QD8z855gZlQFivPyeQf");
          
        }
        

      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http2.errorToString(httpCode2).c_str());
    }
    http2.end();
  }
}
