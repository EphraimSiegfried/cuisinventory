#include <Wire.h>
#include <SerLCD.h>
#include <RTClib.h>


SerLCD lcd;
RTC_PCF8523 rtc;

uint32_t offset = -17; // offset of RTC in seconds

void setup() {
    Wire.begin();
    lcd.begin(Wire);
    Wire.setClock(400000); // set I2C SCL to High Speed Mode of 400kHz

    lcd.setFastBacklight(0xFFFFFF); // set backlight to bright white
    lcd.setContrast(5); // Set contrast. Lower to 0 for higher contrast.
    lcd.clear(); 

    while (!rtc.begin()) {
        lcd.clear();
        lcd.print("Failed to find RTC...");
        delay(200);
    }

    if (!rtc.initialized() || rtc.lostPower()) {
        lcd.clear();
        lcd.print("RTC is not initialized!");
        while (1) delay(10);
    }

    rtc.start();
}

void loop() {
    DateTime now = DateTime(rtc.now().unixtime() - offset);
    lcd.clear();
    lcd.print(now.timestamp());
    delay(100);
}
