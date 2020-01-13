#include "ESPino32CAM.h"
#include "ESPino32CAM_QRCode.h"
ESPino32CAM cam;
ESPino32QRCode qr;
#include <HTTPClient.h>
#include <Arduino.h>
#include <WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <WiFiMulti.h>
WiFiMulti WiFiMulti;
#define BUTTON_QR 0
#define LINE_TOKEN "uWU4WEdevq9ONZtWncVODGkKYlNIgVQ14xm7ro93S2e"
#define Solenoid 23
#define LEDtrue 19
#define LEDred 18
void setup() {
  pinMode(Solenoid, OUTPUT);
  pinMode(LEDtrue, OUTPUT);
  pinMode(LEDred, OUTPUT);
  digitalWrite(23, HIGH);
  Serial.begin(2000000);
  Serial.println("\r\nESPino32CAM");
  if (cam.init() != ESP_OK)
  {
    Serial.println(F("Fail"));
    while (1);
  }
  qr.init(&cam);
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_whitebal(s,true);
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }
  WiFiMulti.addAP("Witawatd", "20102553"); // ssid , password
  randomSeed(50);
}
void loop()
{
  //digitalWrite(LEDtrue, HIGH);
  //delay(1000);
  //digitalWrite(LEDtrue, LOW);
  //delay(1000);
  //digitalWrite(LEDred, HIGH);
  //delay(1000);
  //digitalWrite(LEDred, LOW);
  camera_fb_t *fb = cam.capture();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }
  Serial.write(fb->buf, fb->len);
  dl_matrix3du_t *image_rgb;
  if(cam.jpg2rgb(fb,&image_rgb))
  {
     cam.clearMemory(fb);
    
     cam.printDebug("\r\nQR Read:");
     qrResoult res = qr.recognition(image_rgb);
     WiFiMulti.run();
     delay(1000);
  if(res.status)
  {   
      //String qrdata = res.payload;
      cam.printDebug(res.payload);
      

      if ((WiFiMulti.run() == WL_CONNECTED)) {
    
        HTTPClient http;
    
        String url = "https://iot.rmu.ac.th/iot/Smartlock/checkpassword.php?Password="+String(res.payload);
        //cam.printDebug(url);
        http.begin(url); //HTTP

      int httpCode = http.GET();
      if (httpCode > 0) {
        //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        
        String datapass = http.getString();
        String password = datapass.substring(0,2);
        String namedoor = datapass.substring(3,21);
        String IMEI = datapass.substring(28);
        //cam.printDebug(password);
        //cam.printDebug(namedoor);
        cam.printDebug(datapass);
          if(password=="OK"){
              digitalWrite(LEDtrue, HIGH);
              digitalWrite(Solenoid, LOW);
              String message = "Door:"+String(namedoor)+"\n"+" Status:unlock \n IMEI:"+String(IMEI);
              status("Unlock");
              String messagelock = "Door:"+String(namedoor)+"\n"+" Status:lock \n IMEI:"+String(IMEI);
              Line_Notify(message);
              delay(5000);
              digitalWrite(Solenoid, HIGH);
              Line_Notify(messagelock);
              status("lock");
              
              
              
              
 
            }
           else{
              //digitalWrite(LEDtrue, LOW);
              digitalWrite(LEDred, HIGH);
            
            }
        
      } else {
          digitalWrite(LEDred, HIGH);
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        
        http.end();
    } else{
      
        digitalWrite(LEDred, HIGH);
      }
    
       
  }
  else{
      digitalWrite(LEDtrue, LOW);
      digitalWrite(LEDred, LOW);
      
    }
 
  cam.clearMemory(image_rgb);  
}
}
void status(String data){
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    
    HTTPClient http2;
    
    String url2 = "http://iot.rmu.ac.th/iot/Smartlock/send_esp8266.php?status="+String(data);
    cam.printDebug(url2);
    http2.begin(url2); //HTTP

int httpCode2 = http2.GET();
if (httpCode2 > 0) {
  Serial.printf("[HTTP] GET... code: %d\n", httpCode2);
if (httpCode2 == HTTP_CODE_OK) {
  String payload = http2.getString();
  cam.printDebug(payload);
}
} else {
  Serial.printf("[HTTP] GET... failed, error: %s\n", http2.errorToString(httpCode2).c_str());
}
  http2.end();
}
  }
void Line_Notify(String message) {
 WiFiClientSecure client; // กรณีขึ้น Error ให้ลบ axTLS:: ข้างหน้าทิ้ง

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;   
  }

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Connection: close\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message;
  // Serial.println(req);
  client.print(req);
    
  delay(20);

  // Serial.println("-------------");
  while(client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    //Serial.println(line);
  }
  // Serial.println("-------------");
}
