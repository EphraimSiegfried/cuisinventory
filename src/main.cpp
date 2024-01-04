#include <Adafruit_TinyUSB.h>
#include <BarReader.h>
#include <Constants.h>
#include <DB.h>
#include <Debug.h>
#include <Input.h>
#include <RTClib.h>
#include <SPI.h>
#include <Scale.h>
#include <SerLCD.h>
#include <SparkFun_Qwiic_Button.h>
#include <USB.h>
#include <WiFiService.h>
#include <Wire.h>

SerLCD lcd;
RTC_PCF8523 rtc;
extern QwiicButton greenButton1;
extern QwiicButton greenButton2;
extern QwiicButton redButton;

bool usb = false;             // whether to act as usb mass storage
bool pendingSync = false;     // whether we need to send data to server
uint64_t lastActiveTime = 0;  // millis() of last action

void setup() {
#ifdef DEBUG
    Serial.begin(9600);
#endif
    Wire.begin();
    lcd.begin(Wire);
    Wire.setClock(400000);  // set I2C SCL to High Speed Mode of 400kHz

    lcd.setFastBacklight(0xFFFFFF);  // set backlight to bright white
    lcd.setContrast(
        LCD_CONTRAST);  // Set contrast. Lower to 0 for higher contrast.
    lcd.clear();

    // *** Buttons ***

    while (!greenButton1.begin(GREEN_BUTTON1_ADDRESS)) {
        lcd.clear();
        lcd.print("Green button 1 not found...");
        delay(200);
    }
    while (!greenButton2.begin(GREEN_BUTTON2_ADDRESS)) {
        lcd.clear();
        lcd.print("Green button 2 not found...");
        delay(200);
    }
    while (!redButton.begin(RED_BUTTON_ADDRESS)) {
        lcd.clear();
        lcd.print("Red button not found...");
        delay(200);
    }

    greenButton1.LEDon(BTN_LIGHT_IDLE);
    greenButton2.LEDon(BTN_LIGHT_IDLE);
    redButton.LEDon(BTN_LIGHT_IDLE);

    lcd.clear();

    // *** USB ***

    // hold red button while plugging in to enable usb msc
    if (redButton.isPressed()) {
        lcd.print("** USB MSC activated **");
        usb = true;
        setupUSB();
        return;
    }

    // *** SD ***

    pinMode(SD_PIN, OUTPUT);  // set SD pin mode

    while (!SD.begin(SD_PIN)) {
        lcd.clear();
        lcd.print("Please insert SD card...");
        delay(200);
    }

    lcd.clear();

    // *** Clock ***

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
    lcd.clear();

    // *** Scale ***
    if (!initScale()) {
        lcd.print("Failed to initialize the scale");
        while (1) delay(10);
    };

    // *** Barcode ***
    if (!initBarReader()) {
        lcd.print("Failed to initialize the barcode reader");
        while (1) delay(10);
    };
}

String scanProductBarcode() {
    lcd.clear();
    lcd.setFastBacklight(0x0000FF);
    lcd.print("Please scan product barcode...");
    scanner.startScan();
    String barcode;
    while (!readBar(barcode)) {
        if (input(RED_BUTTON, LONG_PRESS)) return "";  // cancel
        delay(100);
    }
    scanner.stopScan();
    lcd.clear();
    return barcode;
}

uint32_t measureProductWeight() {
    lcd.clear();
    lcd.print("Please weigh product,\npress button 1 to confirm");
    while (!input(GREEN_BUTTON1, SHORT_PRESS)) {
        if (input(RED_BUTTON, LONG_PRESS)) return 0;  // cancel
        delay(100);
    }
    lcd.clear();
    lcd.print("Weighing, please stand by...");
    uint32_t weight = 0;
    while (!(abs(nau.read() - weight) <= STABILITY_THRESHOLD)) {
        weight = nau.read();
        if (input(RED_BUTTON, LONG_PRESS)) return 0;  // cancel
        delay(100);
    }
    lcd.clear();
    return weight;
}

void addProduct() {
    String barcode = scanProductBarcode();
    if (barcode.length() == 0) return;

    uint32_t weight = measureProductWeight();
    if (weight == 0) return;

    StaticJsonDocument<JSONSIZE> doc;
    if (!DB.add(doc, weight, rtc.now().unixtime())) {
        lcd.clear();
        lcd.setFastBacklight(0xFF0000);
        lcd.print("FATAL ERROR:\n");
        lcd.print("Failed to save product");
        while (!input(GREEN_BUTTON1, SHORT_PRESS)) delay(100);
        return;
    }
    pendingSync = true;
    lcd.clear();
    lcd.print("Product added successfully!");
    lcd.setFastBacklight(0x00FF00);
    delay(1000);
    return;
}

void updateProduct() {
    String barcode = scanProductBarcode();
    if (barcode.length() == 0) return;

    uint32_t id = DB.getLeastWeightID(barcode);
    if (id == 0) {
        lcd.clear();
        lcd.setFastBacklight(0xFF0000);
        lcd.print("Please add product first!");
        while (!input(GREEN_BUTTON1, SHORT_PRESS)) delay(100);
        return;
    }
    lcd.clear();

    uint32_t weight = measureProductWeight();
    if (weight == 0) return;

    if (!DB.setWeight(id, weight)) {
        lcd.clear();
        lcd.setFastBacklight(0xFF0000);
        lcd.print("FATAL ERROR:\n");
        lcd.print("Failed to update weight");
        while (!input(GREEN_BUTTON1, SHORT_PRESS)) delay(100);
        return;
    }
    pendingSync = true;
    lcd.clear();
    lcd.print("Weight updated successfully!");
    lcd.setFastBacklight(0x00FF00);
    delay(1000);
    return;
}

void removeProduct() {
    String barcode = scanProductBarcode();
    if (barcode.length() == 0) return;

    uint32_t id = DB.getLeastWeightID(barcode);
    if (id == 0) {
        lcd.clear();
        lcd.setFastBacklight(0xFF0000);
        lcd.print("Product not found in inventory!");
        while (!input(GREEN_BUTTON1, SHORT_PRESS)) delay(100);
        return;
    }
    if (!DB.remove(id, barcode)) {
        lcd.clear();
        lcd.setFastBacklight(0xFF0000);
        lcd.print("FATAL ERROR:\n");
        lcd.print("Failed to remove product");
        while (!input(GREEN_BUTTON1, SHORT_PRESS)) delay(100);
        return;
    }
    pendingSync = true;
    lcd.clear();
    lcd.print("Product removed successfully!");
    lcd.setFastBacklight(0x00FF00);
    delay(1000);
    return;
}

void reset() {
    lastActiveTime = millis();
    lcd.setFastBacklight(0xFFFFFF);
    lcd.clear();
    lcd.print("Cuisinventory ready!");
}

void loop() {
    if (usb) return;  // do nothing if in usb mode
    if (input(GREEN_BUTTON1, SHORT_PRESS)) {
        addProduct();
        reset();
    }
    if (input(GREEN_BUTTON2, SHORT_PRESS)) {
        updateProduct();
        reset();
    }
    if (input(RED_BUTTON, SHORT_PRESS)) {
        removeProduct();
        reset();
    }
    if (pendingSync && (millis() - lastActiveTime >= IDLE_WAIT_BEFORE_SYNC)) {
        lcd.clear();
        lcd.setFastBacklight(0xFFFF00);
        lcd.print("Sync in progress...");
        // if (DB.sync()) pendingSync = false;
        reset();
    }
    delay(100);
}
