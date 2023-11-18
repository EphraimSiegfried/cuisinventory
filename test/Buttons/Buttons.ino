#include <Wire.h>
#include <SerLCD.h> 
#include <SparkFun_Qwiic_Button.h>

enum buttons {
    GREEN_BUTTON1 = 0x6f,
    GREEN_BUTTON2 = 0x70,
    RED_BUTTON = 0x71
};

SerLCD lcd; 
QwiicButton greenButton1;
QwiicButton greenButton2;
QwiicButton redButton;

uint8_t brightness_idle = 10; // brightness of buttons while idle, between 0 and 255
uint8_t brightness_active = 50; // brightness of buttons during click, between 0 and 255


void setup() {
    Wire.begin();
    lcd.begin(Wire);
    Wire.setClock(400000); // set I2C SCL to High Speed Mode of 400kHz

    lcd.setFastBacklight(0xFFFFFF); // set backlight to bright white
    lcd.setContrast(5); // Set contrast. Lower to 0 for higher contrast.
    lcd.clear(); 

    while (!greenButton1.begin(GREEN_BUTTON1)) {
        lcd.clear();
        lcd.print("Green button 1 not found...");
        delay(200);
    }
    while (!greenButton2.begin(GREEN_BUTTON2)) {
        lcd.clear();
        lcd.print("Green button 2 not found...");
        delay(200);
    }
    while (!redButton.begin(RED_BUTTON)) {
        lcd.clear();
        lcd.print("Red button not found...");
        delay(200);
    }

    greenButton1.LEDon(brightness_idle);
    greenButton2.LEDon(brightness_idle);
    redButton.LEDon(brightness_idle);

    lcd.clear();
    lcd.print("Try pushing some buttons...");
}

void handleInput(enum buttons btn) {
    lcd.clear();
    switch (btn) {
        case GREEN_BUTTON1:
            lcd.setFastBacklight(0x00FF00); // green
            lcd.print("Green button 1 was pressed.");
            greenButton1.LEDon(brightness_active);
            while (greenButton1.isPressed()); // wait
            greenButton1.LEDon(brightness_idle);
            break;
        case GREEN_BUTTON2:
            lcd.setFastBacklight(0x0000FF); // blue
            lcd.print("Green button 2 was pressed.");
            greenButton2.LEDon(brightness_active);
            while (greenButton2.isPressed()); // wait
            greenButton2.LEDon(brightness_idle);
            break;
        case RED_BUTTON:
            lcd.setFastBacklight(0xFF0000); // red
            lcd.print("Red button was pressed.");
            redButton.LEDon(brightness_active);
            while (redButton.isPressed()); // wait
            redButton.LEDon(brightness_idle);
            break;
    }
}

void loop() {
    if (greenButton1.isPressed()) handleInput(GREEN_BUTTON1);
    else if (greenButton2.isPressed()) handleInput(GREEN_BUTTON2);
    else if (redButton.isPressed()) handleInput(RED_BUTTON);
    delay(10);
}
