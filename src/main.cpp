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

bool usb = false;             // whether to act as USB mass storage
bool pendingSync = false;     // whether we need to send data to server
bool offlineMode = false;     // whether the user is connected to WiFi
uint64_t lastActiveTime = 0;  // millis() of last action

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else   // __ARM__
extern char* __brkval;
#endif  // __arm__

int freeMemory2() {
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else   // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

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
    LOG(F("start!"));

    // *** Buttons ***

    while (!greenButton1.begin(GREEN_BUTTON1_ADDRESS)) {
        lcd.clear();
        lcd.print(F("Green button 1 not found..."));
        delay(200);
    }
    while (!greenButton2.begin(GREEN_BUTTON2_ADDRESS)) {
        lcd.clear();
        lcd.print(F("Green button 2 not found..."));
        delay(200);
    }
    while (!redButton.begin(RED_BUTTON_ADDRESS)) {
        lcd.clear();
        lcd.print(F("Red button not found..."));
        delay(200);
    }

    greenButton1.LEDon(BTN_LIGHT_IDLE);
    greenButton2.LEDon(BTN_LIGHT_IDLE);
    redButton.LEDon(BTN_LIGHT_IDLE);
    // *** USB ***

    // hold red button while plugging in to enable USB msc
    if (redButton.isPressed()) {
        lcd.print(F("** USB MSC activated **"));
        usb = true;
        setupUSB();
        return;
    }
    // *** SD ***

    pinMode(SD_PIN, OUTPUT);  // set SD pin mode

    while (!SD.begin(SD_PIN)) {
        lcd.clear();
        lcd.print(F("Please insert SD card..."));
        delay(200);
    }

    // *** Clock ***

    while (!rtc.begin()) {
        lcd.clear();
        lcd.print(F("Failed to find RTC..."));
        delay(200);
    }

    if (!rtc.initialized() || rtc.lostPower()) {
        lcd.clear();
        lcd.print(F("RTC is not initialized!"));
        while (1) delay(10);
    }

    rtc.start();
#ifdef DEBUG
    delay(1000);
#endif
    LOG("free mem:");
    LOG(String(freeMemory2()));
    if (!DB.initDatabase()) {
        LOG(F("failed init database"));
    }
    LOG("free mem:");
    LOG(String(freeMemory2()));
    // *** Scale ***
    if (!initScale()) {
        lcd.print(F("Failed to initialize the scale"));
        while (1) {
            LOG(F("fail scale"));
            delay(1000);
        }
    }

    // *** Barcode ***
    if (!initBarReader()) {
        lcd.print(F("Failed to initialize the barcode reader"));
        while (1) {
            LOG(F("fail barcode"));
            delay(1000);
        }
    }

    // helper lambda to handle wifi-setup errors
    auto enterOfflineMode = [&](const String& errorMessage) {
        LOG(errorMessage);
        lcd.print(errorMessage);
        lcd.print(F("\nEntering offline mode"));
        delay(2000);
        lcd.clear();
        offlineMode = true;
    };

    if (!WiFiService.connect(F("iPhone"), F("12345678"))) {
        enterOfflineMode(F("Failed to connect to Wi-Fi"));
        return;
    }

    LOG(F("all initialized!"));
    lcd.setFastBacklight(0xFFFFFF);
    lcd.clear();
    lcd.print(F("Cuisinventory ready!"));
}

void printError(String errorMessage) {
    lcd.clear();
    lcd.setFastBacklight(0xFF0000);
    LOG(errorMessage);
    lcd.print(errorMessage);
    // wait for confirmation
    while (!input(GREEN_BUTTON1, SHORT_PRESS)) delay(100);
}

void printSuccess(String successMessage) {
    lcd.clear();
    lcd.setFastBacklight(0x00FF00);
    lcd.print(successMessage);
    delay(2000);
}

void printInfo(String infoMessage) {
    lcd.clear();
    lcd.setFastBacklight(0x0000FF);
    lcd.print(infoMessage);
    // delay(2000);
}

String scanProductBarcode() {
    printInfo(F("Please scan barcode..."));
    scanner.startScan();
    String barcode;
    while (!readBar(barcode)) {
        scanner.startScan();
        if (input(RED_BUTTON, LONG_PRESS)) {
            scanner.stopScan();
            LOG(F("abort scan"));
            return "";  // cancel
        }
        delay(100);
    }
    scanner.stopScan();
    return barcode;
}

uint32_t measureProductWeight() {
    // printInfo(
    //     "Please put the product on the scale,\nPress button 1 to confirm");
    printInfo(F("now weigh"));
    LOG(F("weigh"));
    while (!input(GREEN_BUTTON1, SHORT_PRESS)) {
        if (input(RED_BUTTON, LONG_PRESS)) return 0;  // cancel
        delay(100);
    }
    printInfo(F("Weighing, please stand by...\n"));
    uint32_t weight;
    do {
        lcd.clear();
        weight = nau.read();
        lcd.print(String(weight / SCALING));
        if (input(RED_BUTTON, LONG_PRESS)) return 0;  // cancel
        delay(100);
    } while (!(abs(nau.read() - weight) <= STABILITY_THRESHOLD));
    // get average
    for (uint8_t i; i < SAMPLE_AMOUNT; i++) {
        weight += nau.read();
    }
    weight = (weight / (SAMPLE_AMOUNT - 1)) / SCALING;
    return weight;
}

void addProduct() {
    LOG(F("please scan barcode to add"));
    // lcd.print("lol");
    // printInfo("Please scan barcode to add product...");
    String barcode = scanProductBarcode();
    if (barcode.length() == 0) return;

    uint32_t weight = measureProductWeight();
    if (weight == 0) return;

    LOG(F("barcode: "));
    LOG(barcode);
    JsonDocument doc;
    if (!WiFiService.get(barcode.c_str(), doc)) {
        printError(F("FATAL ERROR\nFailed to get product info"));
        return;
    }
    LOG(F("WiFi get done"));

    if (!DB.add(doc, weight, rtc.now().unixtime())) {
        printError(F("FATAL ERROR\nFailed to save product"));
        return;
    }
    LOG(F("DB add done"));

    pendingSync = true;
    printSuccess(F("Product added successfully!"));
    return;
}

void updateProduct() {
    LOG(F("please scan barcode to update"));
    // printInfo("Please scan barcode to update product weight...");
    String barcode = scanProductBarcode();
    if (barcode.length() == 0) return;

    uint32_t id = DB.getLeastWeightID(barcode);
    if (id == 0) {
        printError(F("Please add product first!"));
        return;
    }

    uint32_t weight = measureProductWeight();
    if (weight == 0) return;

    if (!DB.setWeight(id, weight)) {
        printError(F("FATAL ERROR\nFailed to update weight"));
        return;
    }
    printSuccess(F("Weight updated successfully!"));
    return;
}

void removeProduct() {
    LOG(F("please scan barcode to remove"));
    // printInfo("Please scan barcode to remove product...");
    String barcode = scanProductBarcode();
    if (barcode.length() == 0) return;

    uint32_t id = DB.getLeastWeightID(barcode);
    if (id == 0) {
        printError(F("Product not found in inventory!"));
        return;
    }
    if (!DB.remove(id, barcode)) {
        printError(F("FATAL ERROR\nFailed to remove product"));
        return;
    }
    printSuccess(F("Product removed successfully!"));
    return;
}

void printProducts() {
    std::vector<uint32_t> ids = DB.getAllIDs();
    if (ids.empty()) {
        printError(F("The inventory is empty!"));
        return;
    }

    JsonDocument productJson;
    char dateformat[] = "DD.MM.YY: hh:mm";

    auto printProd = [&](size_t i, JsonDocument& doc) {
        DB.getJsonFromID(ids[i], doc);
        char* date = DateTime(doc["date"].as<uint32_t>()).toString(dateformat);
        String name = doc["name"].as<String>();
        String current = String(i) + "/" + String(ids.size());
        printInfo(current + F("\n") + name + F("\n") + F("Enter date: ") +
                  date);
    };

    size_t i = ids.size() - 1;
    printProd(ids[i], productJson);

    while (1) {
        if (input(GREEN_BUTTON1, SHORT_PRESS)) {  // forwards
            i = (i + 1) % ids.size();
            printProd(i, productJson);
        }
        if (input(GREEN_BUTTON2, SHORT_PRESS)) {  // backwards
            i = (i == 0) ? ids.size() - 1 : i - 1;
            printProd(i, productJson);
        }
        if (input(RED_BUTTON, LONG_PRESS)) return;  // cancel
        delay(100);
    }
}
void reset() {
    lastActiveTime = millis();
    lcd.setFastBacklight(0xFFFFFF);
    lcd.clear();
    lcd.print(F("Cuisinventory ready!"));
}

void loop() {
    if (usb) return;  // do nothing if in USB mode
    // LOG("tick");
    if (input(GREEN_BUTTON1, SHORT_PRESS)) {
        LOG(F("green 1 short"));
        addProduct();
        reset();
    }
    if (input(GREEN_BUTTON2, SHORT_PRESS)) {
        LOG(F("green 2 short"));
        updateProduct();
        reset();
    }
    if (input(RED_BUTTON, SHORT_PRESS)) {
        LOG(F("red short"));
        removeProduct();
        reset();
    }
    if (input(GREEN_BUTTON1, LONG_PRESS)) {
        LOG(F("green 1 long"));
        printProducts();
        reset();
    }
    if (!offlineMode && pendingSync &&
        (millis() - lastActiveTime >= IDLE_WAIT_BEFORE_SYNC)) {
        lcd.clear();
        lcd.setFastBacklight(0xFFFF00);
        lcd.print(F("Sync in progress..."));
        if (DB.syncDB()) pendingSync = false;
        reset();
    }
    delay(100);
}
