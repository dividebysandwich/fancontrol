#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>

#define DHTPIN_1 25 // The inside sensor is on GPIO 25
#define DHTPIN_2 26 // The outside sensor is on GPIO 26

const int RELAYPIN_1 = 5; // The intake fan relay is on GPIO 5
const int RELAYPIN_2 = 13; // The extractor fan relay is on GPIO 13

int loopcount;
bool fan_in_active;
bool fan_out_active;

DHTesp dht_inside;
DHTesp dht_outside;
LiquidCrystal_I2C lcd(0x3F,20,4); // Set the LCD address to 0x3F for a 20 chars and 4 line display (some displays are on address 0x27 instead)

void setup() {
  loopcount = 29;
  fan_in_active = false;
  fan_out_active = false;
  Serial.begin(115200);
  Serial.print("Starting...");
  delay(500);
  pinMode(RELAYPIN_1, OUTPUT);
  digitalWrite(RELAYPIN_1, HIGH);
  pinMode(RELAYPIN_2, OUTPUT);
  digitalWrite(RELAYPIN_2, HIGH);
  lcd.begin(27, 14);// initialize the lcd with SDA and SCL pins 27 and 14 respectively
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Fan Control");
  lcd.setCursor(0,1);
  lcd.print("(c) Josef Jahn");
  dht_inside.setup(DHTPIN_1, DHTesp::DHT22);
  delay(1100);
  dht_outside.setup(DHTPIN_2, DHTesp::DHT22);
  delay(1100);
}

void loop() {
  loopcount++;
  float h_inside = dht_inside.getHumidity();
  float t_inside = dht_inside.getTemperature();
  float h_outside = dht_outside.getHumidity();
  float t_outside = dht_outside.getTemperature();

  Serial.println("Outdoor temp:");
  Serial.println(t_outside);
  Serial.println("Outdoor humidity:");
  Serial.println(h_outside);
  Serial.println("Indoor temp:");
  Serial.println(t_inside);
  Serial.println("Indoor humidity:");
  Serial.println(h_inside);

  // display temperature and humidity data
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Outdoor:");
  lcd.setCursor(0,2);
  lcd.print(String(t_outside,1));
  lcd.setCursor(0,3);
  lcd.print(String(h_outside,1));

  lcd.setCursor(5,2);
  lcd.print((char)223);
  lcd.setCursor(6,2);
  lcd.print("C");
  lcd.setCursor(5,3);
  lcd.print("%");

  lcd.setCursor(13,0);
  lcd.print("Indoor:");
  lcd.setCursor(12,2);
  lcd.print(String(t_inside,1));
  lcd.setCursor(12,3);
  lcd.print(String(h_inside,1));

  lcd.setCursor(18,2);
  lcd.print((char)223);
  lcd.setCursor(19,2);
  lcd.print("C");
  lcd.setCursor(18,3);
  lcd.print("%");

  // Check if any sensors failed
  if (isnan(h_inside) || isnan(t_inside) || isnan(h_outside) || isnan(t_outside)) {
    Serial.println("Sensor Error!");
    lcd.setCursor(0,1);
    lcd.print("ERROR: SENSOR FAILED");
    delay(3000);
    ESP.restart();
    return;
  }

  // If the inside temperature is higher than the threshold, extract the hot air.
  if (t_inside > 27.0) {
    lcd.setCursor(17,1);
    lcd.print("==>");
    fan_in_active = true;
  } else {
    fan_in_active = false;
  }

  // The intake fan only pulls in fresh air if the humidity outside is lower, and the temperature differential is not too large.
  if (h_inside > h_outside && t_outside < t_inside + 8.0 && t_outside > 15.0) {
    lcd.setCursor(0,1);
    lcd.print("==>");
    fan_out_active = true;
  } else {
    fan_out_active = false;
  }

  // Every 30 loops, toggle the fan if needed. This prevents rapid oscillation.
  if (loopcount > 30) {
    loopcount = 0;
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

  }

  delay(3000);
}
