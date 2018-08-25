#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "SparkFun_Si7021_Breakout_Library.h"

const int RELAYPIN_1 = 5; // The intake fan relay is on GPIO 5
const int RELAYPIN_2 = 13; // The extractor fan relay is on GPIO 13
const int RESETPIN = 32; // Reset button
#define TCAADDR 0x70 // I2C Multiplexer Address

const char* ssid     = ""; //Your wifi access data
const char* password = "";

const char* host = "";  //Put the hostname for where the backend data files are accessible here
const char* weatherhost = "";  //Put the hostname for where external weather data can be accessed

bool fan_in_active;
bool fan_out_active;
unsigned long lastSendTime = 0;
float t_outside = 0.0;
float h_outside = 0.0;

Weather sensor_inside; 
LiquidCrystal_I2C lcd(0x3F,20,4); // Set the LCD address to 0x3F for a 20 chars and 4 line display (some displays are on address 0x27 instead)

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
  delay(10);
}

bool readPastHeader(WiFiClient *pClient)
{
 bool bIsBlank = true;
 while(true)
 {
   if (pClient->available()) 
   {
     char c = pClient->read();
     if(c=='\r' && bIsBlank)
     {
       // throw away the /n
       c = pClient->read();
       return true;
     }
     if(c=='\n')
       bIsBlank = true;
     else if(c!='\r')
       bIsBlank = false;
     }
  }
}

void wifiReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setup() {
  delay(100);
  fan_in_active = false;
  fan_out_active = false;
  Serial.begin(115200);
  Serial.print("Starting...");
  delay(100);
  pinMode(RELAYPIN_1, OUTPUT);
  digitalWrite(RELAYPIN_1, HIGH);
  pinMode(RELAYPIN_2, OUTPUT);
  digitalWrite(RELAYPIN_2, HIGH);
  pinMode(RESETPIN, INPUT_PULLUP);
  delay(1000);
  digitalWrite(RELAYPIN_1, LOW);
  digitalWrite(RELAYPIN_2, LOW);
  delay(200);
  tcaselect(0);
  delay(200);
  lcd.begin(27, 14);// initialize the lcd with SDA and SCL pins 27 and 14 respectively
  lcd.backlight();  
  lcd.setCursor(0,0);
  lcd.print("Fan Control");
  if (WiFi.status() != WL_CONNECTED) {
    wifiReconnect();
  }
  lcd.setCursor(0,1);
  lcd.print("(c) Josef Jahn");
  tcaselect(1);
  sensor_inside.begin();
  delay(3000);
  digitalWrite(RELAYPIN_1, HIGH);
  digitalWrite(RELAYPIN_2, HIGH);
  tcaselect(0);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Outdoor:");

  lcd.setCursor(13,0);
  lcd.print("Indoor:");
}

void loop() {
  if (digitalRead(RESETPIN) == LOW)
  { // Check if button has been pressed
    while (digitalRead(RESETPIN) == LOW) {
      // Wait for button to be released
    }
    ESP.restart();
  }
  tcaselect(1);
  float h_inside = sensor_inside.getRH(); ;
  float t_inside = sensor_inside.getTemp();

//  tcaselect(2);
//  float h_outside = sensor_outside.getRH();
//  float t_outside = sensor_outside.getTemp();

  Serial.println("Outdoor temp:");
  Serial.println(t_outside);
  Serial.println("Outdoor humidity:");
  Serial.println(h_outside);
  Serial.println("Indoor temp:");
  Serial.println(t_inside);
  Serial.println("Indoor humidity:");
  Serial.println(h_inside);

  // display temperature and humidity data
  tcaselect(0);
  lcd.setCursor(0,2);
  lcd.print(String(t_outside,2));
  lcd.setCursor(0,3);
  lcd.print(String(h_outside,2));

  lcd.setCursor(5,2);
  lcd.print(' ');
  lcd.setCursor(6,2);
  lcd.print((char)223);
  lcd.setCursor(7,2);
  lcd.print("C");
  lcd.setCursor(5,3);
  lcd.print(" %");

  lcd.setCursor(12,2);
  lcd.print(String(t_inside,2));
  lcd.setCursor(12,3);
  lcd.print(String(h_inside,2));

  lcd.setCursor(17,2);
  lcd.print(' ');
  lcd.setCursor(18,2);
  lcd.print((char)223);
  lcd.setCursor(19,2);
  lcd.print("C");
  lcd.setCursor(18,3);
  lcd.print("%");

  if (!isnan(t_inside)) {
    // If the inside temperature is higher than the threshold, extract the hot air.
    if (t_inside > 27.0) {
      lcd.setCursor(17,1);
      lcd.print("==>");
      fan_in_active = true;
    } else {
      lcd.setCursor(17,1);
      lcd.print("   ");
      fan_in_active = false;
    }
  }
  
  if (!isnan(t_outside) && !isnan(t_outside) && h_outside != 0.0) {
    // The intake fan only pulls in fresh air if the humidity outside is dry enough, and the temperature differential is not too large.
    if ((h_outside <= 55.0 || h_outside < h_inside) && t_outside < t_inside + 4.0 && t_outside > 10.0) {
      lcd.setCursor(0,1);
      lcd.print("==>");
      fan_out_active = true;
    } else {
      lcd.setCursor(0,1);
      lcd.print("   ");
      fan_out_active = false;
    }
  }
  
  unsigned long t = millis();

  //millis() does a rollover
  if (t < lastSendTime) {
    lastSendTime = t;
  }
  
  if (lastSendTime + 60000 < t || lastSendTime == 0) { 
    lastSendTime = t;

    if (fan_in_active) {
      digitalWrite(RELAYPIN_1, LOW);
    } else {
      digitalWrite(RELAYPIN_1, HIGH);
    }

    if (fan_out_active) {
      digitalWrite(RELAYPIN_2, LOW);
    } else {
      digitalWrite(RELAYPIN_2, HIGH);
    }

    if (WiFi.status() != WL_CONNECTED) {
      wifiReconnect();
      delay(1000);
    }


    Serial.println("Sending Data");
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        delay(3000);
        return;
    }

    // This will send the request to the server
    client.print(String("GET ") + "/status/climate_backend.php?ti="+String(t_inside,2)+"&to="+String(t_outside,2)+"&hi="+String(h_inside,2)+"&ho="+String(h_outside,2)+" HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            break;
        }
    }
    client.stop();
   
    if (!client.connect(weatherhost, httpPort)) {
        Serial.println("connection failed");
        delay(3000);
        return;
    }

    client.print(String("GET ") + "/lasttemperature.txt" + " HTTP/1.1\r\n" +
                 "Host: " + weatherhost + "\r\n" +
                 "Connection: Keep-Alive \r\n\r\n");
    timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    if (client.available()) {
        readPastHeader(&client);
        t_outside = client.readStringUntil('\n').toFloat();
    }

    client.print(String("GET ") + "/lasthumid.txt" + " HTTP/1.1\r\n" +
                 "Host: " + weatherhost + "\r\n" +
                 "Connection: close\r\n\r\n");
    timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    if (client.available()) {
        readPastHeader(&client);
        h_outside = client.readStringUntil('\n').toFloat();
    }
    client.stop();
  }

  delay(200);
}
