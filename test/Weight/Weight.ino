#include <Wire.h>
#include <SerLCD.h>
#include <Adafruit_NAU7802.h>

SerLCD lcd;
Adafruit_NAU7802 nau;

void setup() {
    Wire.begin();
    lcd.begin(Wire);

    Wire.setClock(400000);

    lcd.setFastBacklight(0xFFFFFF);

    while (!nau.begin()) {
        lcd.print("NAU7902 not found...");
        delay(200);
    }
    
    nau.setLDO(NAU7802_3V0);
    lcd.clear();
    lcd.print("LDO voltage set to ");
    switch (nau.getLDO()) {
        case NAU7802_4V5:  lcd.print("4.5V"); break;
        case NAU7802_4V2:  lcd.print("4.2V"); break;
        case NAU7802_3V9:  lcd.print("3.9V"); break;
        case NAU7802_3V6:  lcd.print("3.6V"); break;
        case NAU7802_3V3:  lcd.print("3.3V"); break;
        case NAU7802_3V0:  lcd.print("3.0V"); break;
        case NAU7802_2V7:  lcd.print("2.7V"); break;
        case NAU7802_2V4:  lcd.print("2.4V"); break;
        case NAU7802_EXTERNAL:  lcd.print("External"); break;
    }

    nau.setGain(NAU7802_GAIN_128);
    lcd.print("\nGain set to ");
    switch (nau.getGain()) {
        case NAU7802_GAIN_1:  lcd.print("1x"); break;
        case NAU7802_GAIN_2:  lcd.print("2x"); break;
        case NAU7802_GAIN_4:  lcd.print("4x"); break;
        case NAU7802_GAIN_8:  lcd.print("8x"); break;
        case NAU7802_GAIN_16:  lcd.print("16x"); break;
        case NAU7802_GAIN_32:  lcd.print("32x"); break;
        case NAU7802_GAIN_64:  lcd.print("64x"); break;
        case NAU7802_GAIN_128:  lcd.print("128x"); break;
    }

    nau.setRate(NAU7802_RATE_10SPS);
    lcd.print("\nConversion rate set to ");
    switch (nau.getRate()) {
        case NAU7802_RATE_10SPS:  lcd.print("10 SPS"); break;
        case NAU7802_RATE_20SPS:  lcd.print("20 SPS"); break;
        case NAU7802_RATE_40SPS:  lcd.print("40 SPS"); break;
        case NAU7802_RATE_80SPS:  lcd.print("80 SPS"); break;
        case NAU7802_RATE_320SPS:  lcd.print("320 SPS"); break;
    }

    // Take 10 readings to flush out readings
    for (uint8_t i=0; i<10; i++) {
        while (! nau.available()) delay(1);
        nau.read();
    }

    delay(3000);

    while (!nau.calibrate(NAU7802_CALMOD_INTERNAL)) {
        lcd.print("Failed to calibrate internal offset, retrying!");
        delay(1000);
    }
    lcd.clear();
    lcd.print("Calibrated internal offset");

    while (! nau.calibrate(NAU7802_CALMOD_OFFSET)) {
        lcd.clear();
        lcd.print("Failed to calibrate system offset, retrying!");
        delay(1000);
    }
    lcd.clear();
    lcd.print("Calibrated system offset");
}

void loop() {
    while (!nau.available()) {
        delay(1);
    }
    int32_t val = nau.read();
    lcd.clear();
    lcd.print("Read ");
    lcd.print(val);
    delay(500);
}
