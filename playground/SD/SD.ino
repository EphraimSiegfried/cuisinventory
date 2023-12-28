#include <SD.h>
#include <Wire.h>
#include <SerLCD.h>


SerLCD lcd;
File testFile;

const int CSpin = 10; // SD pin

void setup() {
    Wire.begin();
    lcd.begin(Wire);
    Wire.setClock(400000); // set I2C SCL to High Speed Mode of 400kHz

    lcd.setFastBacklight(0xFFFFFF); // set backlight to bright white
    lcd.setContrast(5); // Set contrast. Lower to 0 for higher contrast.
    lcd.clear(); 

    pinMode(CSpin, OUTPUT); // set SD pin mode

    while (!SD.begin(CSpin)) {
        lcd.clear();
        lcd.print("Please insert SD card...");
        delay(200);
    }

    lcd.clear();

    // try to display contents of test.txt
    if (SD.exists("test.txt")) {
        testFile = SD.open("test.txt", FILE_READ);
        if (testFile) {
            lcd.println("** File contents **");
            // read char by char
            while (testFile.available()) {
                lcd.write(testFile.read());
            }
            testFile.close();
        }
    } else {
        lcd.print("Creating test.txt...");
        testFile = SD.open("test.txt", FILE_WRITE);
        if (testFile) {
            testFile.println("Hello file!");
            testFile.close();
        }
    }
}

void loop() {

}
