#include <Wire.h>
#include <SerLCD.h>

SerLCD lcd;

void setup() {
    Wire.begin();
    lcd.begin(Wire);

    Wire.setClock(400000);

    lcd.print("Testing Set RGB");
    delay(2000);
}

void loop() {
    //Turn off backlight (black)
    lcd.setFastBacklight(0, 0, 0);
    lcd.clear();
    lcd.print("Black (Off)!");

    delay(2000);


    //Set red backlight
    lcd.setFastBacklight(255, 0, 0);
    lcd.clear();
    lcd.print("Red!");

    delay(2000);


    //Set orange backlight
    lcd.setFastBacklight(255, 140, 0);
    lcd.clear();
    lcd.print("Orange!");

    delay(2000);


    //Set yellow backlight
    lcd.setFastBacklight(255, 255, 0);
    lcd.clear();
    lcd.print("Yellow!");

    delay(2000);


    //Set green backlight
    lcd.setFastBacklight(0, 255, 0);
    lcd.clear();
    lcd.print("Green!");

    delay(2000);


    //Set blue backlight
    lcd.setFastBacklight(0, 0, 255);
    lcd.clear();
    lcd.print("Blue!");

    delay(2000);


    //Set violet backlight
    lcd.setFastBacklight(160, 32, 240);
    lcd.clear();
    lcd.print("Violet!");

    delay(2000);


    //Turn on all (white)
    lcd.setFastBacklight(255, 255, 255);
    lcd.clear();
    lcd.print("White!");
    delay(2000);


    //Set to Gray
    lcd.setFastBacklight(128, 128, 128);
    lcd.clear();
    lcd.print("Gray!");
    delay(2000);
}
