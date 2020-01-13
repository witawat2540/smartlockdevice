
#include <ESP8266WiFi.h>
#include <MicroGear.h>

const char* ssid     = "Witawatd";
const char* password = "20102553";

#define APPID   "WorkShopLED"
#define KEY     "tjVAVjlbJHII6nq"
#define SECRET  "MgBnXJHN74Lm6ISZ3m3oogCSO" 

#define ALIAS   "smartlock"

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

void setup(){


    microgear.on(MESSAGE,onMsghandler);
    microgear.on(CONNECTED,onConnected);

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

    microgear.init(KEY,SECRET,ALIAS);
    microgear.connect(APPID);
}

void loop(){
    if (microgear.connected()) {
        Serial.println("connected");
        microgear.loop();

        if (timer >= 1000) {
       
        String a = "dfsadfsa,nkjdsnfids";

            Serial.print("Sending -->");
            Serial.print(a);
            microgear.publish("/door",String(a));

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
