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
void setup() {
  pinMode(Solenoid, OUTPUT);
  pinMode(LEDtrue, OUTPUT);
  pinMode(18, OUTPUT);
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
  {   digitalWrite(LEDtrue, HIGH);
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
              digitalWrite(Solenoid, LOW);
              String message = "Door:"+String(namedoor)+"\n"+" Status:unlock \n IMEI:"+String(IMEI);
              String messagelock = "Door:"+String(namedoor)+"\n"+" Status:lock \n IMEI:"+String(IMEI);
              Line_Notify(message);
              delay(5000);
              Line_Notify(messagelock);
              
 
            }
           else{
              digitalWrite(LEDtrue, LOW);
            
            }
        
      } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
      http.end();
    }
       
  }
  else{
      
      digitalWrite(LEDtrue, LOW);
      digitalWrite(Solenoid, HIGH);
      
      
    }
 
  cam.clearMemory(image_rgb);  
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
