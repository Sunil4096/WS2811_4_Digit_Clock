#include <Adafruit_NeoPixel.h>  // WS2811 control via **Adafruit_NeoPixel**
#include "RTClib.h"             // DS1307 RTC handling via **RTClib by Adafruit** Written by JeeLabs MIT license

RTC_DS1307 rtc;

#define LED_PIN 6
#define LED_COUNT 50  // 7 LEDs = 1 digit
#define Decimal_led 2
#define Digit_leds 7  // how many leds on each digit
#define BT_UP 12      // up button
#define BT_OK 11      // center button
#define BT_DWN 10     // down button

Adafruit_NeoPixel LEDs(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

uint32_t old_s;
bool led_state;
uint8_t mode;
uint8_t setHour;
int setMinute;
uint8_t brightness_ = 55;
uint32_t background_color_ = LEDs.Color(22, 22, 22);
uint32_t huor_color = LEDs.Color(255, 33, 33);
uint32_t minute_color = LEDs.Color(33, 255, 33);

// Segment map for digits 0–9
// Order: A B C D E F G
const uint8_t digitMap[10][7] = {
  { 1, 1, 1, 1, 1, 1, 0 },  // 0
  { 0, 1, 1, 0, 0, 0, 0 },  // 1
  { 1, 1, 0, 1, 1, 0, 1 },  // 2
  { 1, 1, 1, 1, 0, 0, 1 },  // 3
  { 0, 1, 1, 0, 0, 1, 1 },  // 4
  { 1, 0, 1, 1, 0, 1, 1 },  // 5
  { 1, 0, 1, 1, 1, 1, 1 },  // 6
  { 1, 1, 1, 0, 0, 0, 0 },  // 7
  { 1, 1, 1, 1, 1, 1, 1 },  // 8
  { 1, 1, 1, 1, 0, 1, 1 }   // 9
};

void setup() {
  pinMode(BT_UP, INPUT_PULLUP);
  pinMode(BT_OK, INPUT_PULLUP);
  pinMode(BT_DWN, INPUT_PULLUP);
  // ________________________________________________________________
  LEDs.begin();
  LEDs.setBrightness(brightness_ * 2.5);  // We can set the brightness up to 255
  LEDs.clear();
  LEDs.show();
  // ________________________________________________________________
  Serial.begin(57600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2026, 1, 17, 3, 0, 0));
  }
  // ________________________________________________________________
  DateTime now = rtc.now();
  setHour = now.twelveHour();
  setMinute = now.minute();
}

void loop() {
  DateTime now = rtc.now();

  if (digitalRead(BT_OK) == 0) {
    while (digitalRead(BT_OK) == 0) {
      delay(10);
    }
    mode++;
    delay(150);
  }
  if (mode >= 4) {
    mode = 0;
  }
  if (mode == 0) {
    setHour = now.twelveHour();
    setMinute = now.minute();
  }

  // ________________________________________________________________
  if (now.unixtime() != old_s || mode != 0) {
    LEDs.clear();
    if (mode == 1) {  // huor set mode
      if (digitalRead(BT_UP) == 0) {
        setHour++;
        if (setHour > 12) { setHour = 1; }
      }
      if (digitalRead(BT_DWN) == 0) {
        setHour--;
        if (setHour < 1) { setHour = 12; }
      }
      showDigit_H(setHour, LEDs.Color(255, 0, 0), background_color_);
      delay(150);
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), setHour, now.minute(), now.second()));
      DateTime now = rtc.now();
      setHour = now.twelveHour();
    }

    // ________________________________________________________________
    if (mode == 2) {  // minute set mode
      if (digitalRead(BT_UP) == 0) {
        setMinute++;
        if (setMinute > 59) { setMinute = 0; }
      }
      if (digitalRead(BT_DWN) == 0) {
        setMinute--;
        if (setMinute < 0) { setMinute = 59; }
      }
      showDigit_M(setMinute, LEDs.Color(255, 0, 0), background_color_);
      delay(150);
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.twelveHour(), setMinute, now.second()));
      DateTime now = rtc.now();
      setMinute = now.minute();
    }

    // ________________________________________________________________
    if (mode == 3) {  // mode 3 is brightness adjust
      if (digitalRead(BT_UP) == 0) {
        brightness_++;
        delay(100);
        if (brightness_ > 99) {
          brightness_ = 5;
        }
      }
      if (digitalRead(BT_DWN) == 0) {
        brightness_--;
        delay(100);
        if (brightness_ < 5) {
          brightness_ = 99;
        }
      }
      // uint8_t bright = map(brightness_, 0, 99, 0, 255);
      LEDs.setBrightness(brightness_ * 2.5);
      showDigit_M(brightness_, LEDs.Color(255, 0, 0), background_color_);  // for brightness Disdisplay
      for (int i = 2; i < 7; i++) {
        LEDs.setPixelColor(i, LEDs.Color(255, 0, 0));
      }
    }

    // __________________show normal time______________________________________________
    if (mode == 0 || mode == 2) {
      showDigit_H(now.twelveHour(), huor_color, background_color_);
    }
    if (mode == 0 || mode == 1) {
      showDigit_M(now.minute(), minute_color, background_color_);
    }
    delay(10);

    // ________________________________________________________________
    led_state = !led_state;
    LED_blink(14, led_state);
    LED_blink(15, led_state);
    // ________________________________________________________________
    LEDs.show();  // Displays the color status of all LEDs.
    // ________________________________________________________________
    Serial.print(now.twelveHour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);

    Serial.print("   set ");
    Serial.print(setHour);
    Serial.print(":");
    Serial.print(setMinute);
    Serial.print("      ");

    Serial.print(" BT_UP = ");
    Serial.print(digitalRead(BT_UP));
    Serial.print(" BT_OK = ");
    Serial.print(digitalRead(BT_OK));
    Serial.print(" BT_DWN = ");
    Serial.print(digitalRead(BT_DWN));
    Serial.print(" mode = ");
    Serial.print(mode);
    Serial.println();
    // ________________________________________________________________
    old_s = now.unixtime();
  }
}


//________________Hours_________
void showDigit_H(uint8_t num, uint32_t color, uint32_t background_color) {
  uint32_t num_1 = num / 10;
  uint32_t num_2 = num % 10;
  if (num_1 != 0) {
    for (int i = 0; i < 7; i++) {  // digit + 1
      if (digitMap[num_1][i]) {
        LEDs.setPixelColor(i, color);
      } else {
        LEDs.setPixelColor(i, background_color);  // OFF
      }
    }
  }
  for (int i = 0; i < 7; i++) {  // digit + 2
    if (digitMap[num_2][i]) {
      LEDs.setPixelColor(i + 7, color);
    } else {
      LEDs.setPixelColor(i + 7, background_color);  // OFF   // OFF
    }
  }
}

//________________Minutes_________
void showDigit_M(uint8_t num, uint32_t color, uint32_t background_color) {
  uint32_t num_1 = num / 10;
  uint32_t num_2 = num % 10;
  if (num_1 != 0) {
    for (int i = 0; i < 7; i++) {  // digit + 3
      if (digitMap[num_1][i]) {
        LEDs.setPixelColor(i + 16, color);
      } else {
        LEDs.setPixelColor(i + 16, background_color);  // OFF
      }
    }
  }
  for (int i = 0; i < 7; i++) {  // digit + 4
    if (digitMap[num_2][i]) {
      LEDs.setPixelColor(i + 7 + 16, color);
    } else {
      LEDs.setPixelColor(i + 7 + 16, background_color);  // OFF
    }
  }
}

//________________LED blink_________
void LED_blink(int led_address, uint8_t led_state_) {
  // LEDs.setPixelColor(led_address, LEDs.Color(random(55, 255), random(55, 255), random(55, 255)));
  if (led_state_) {
    LEDs.setPixelColor(led_address, LEDs.Color(0, 0, 255));
  } else {
    LEDs.setPixelColor(led_address, LEDs.Color(0, 0, 0));
  }
}
