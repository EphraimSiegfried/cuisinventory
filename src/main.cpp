#include <SD.h>
#include <Wire.h>
#include <SerLCD.h>
#include <RTClib.h>
#include "Constants.h"
#include "DB.h"
#include <Adafruit_TinyUSB.h>
#include <SparkFun_Qwiic_Button.h>

#define _TASK_STATUS_REQUEST     // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_LTS_POINTER        // Compile with support for local task storage pointer
//#include <TaskScheduler.h>

//Scheduler ts;

enum buttons {
    GREEN_BUTTON1 = 0x6f,
    GREEN_BUTTON2 = 0x70,
    RED_BUTTON = 0x71
};

SerLCD lcd;
Sd2Card card;
SdVolume volume;
RTC_PCF8523 rtc;
QwiicButton greenButton1;
QwiicButton greenButton2;
QwiicButton redButton;
//Adafruit_USBD_MSC usb_msc;

bool usb = false; // whether to act as usb mass storage

//Task tInput(T_INPUT_INTERVAL * TASK_MILLISECOND, TASK_FOREVER, &inputCB, &ts, true);

void setup() {
#ifdef DEBUG
    Serial.begin(9600);
#endif
    Wire.begin();
    lcd.begin(Wire);
    Wire.setClock(400000); // set I2C SCL to High Speed Mode of 400kHz

    lcd.setFastBacklight(0xFFFFFF); // set backlight to bright white
    lcd.setContrast(LCD_CONTRAST); // Set contrast. Lower to 0 for higher contrast.
    lcd.clear(); 

    // *** Buttons ***

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

    greenButton1.LEDon(BTN_LIGHT_IDLE);
    greenButton2.LEDon(BTN_LIGHT_IDLE);
    redButton.LEDon(BTN_LIGHT_IDLE);

    lcd.clear();

    // *** USB ***

    // hold red button while plugging in to enable usb msc
    if (redButton.isPressed()) {
        lcd.print("** USB MSC activated **");
        usb = true;
//        setupUSB();
        return;
    }

    // *** SD ***

    pinMode(SD_PIN, OUTPUT); // set SD pin mode

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
}

void loop() {
    if (usb) return; // do nothing if in usb mode
}

void inputCB() {

}

/*void setupUSB() {
    // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
    usb_msc.setID("Vendor", "Cuisinventory", "1.0");

    // Set read write callback
    usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

    // Still initialize MSC but tell usb stack that MSC is not ready to read/write
    // If we don't initialize, board will be enumerated as CDC only
    usb_msc.setUnitReady(false);
    usb_msc.begin();

    if (!card.init(SPI_HALF_SPEED, SD_PIN)) {
        lcd.clear();
        lcd.print("Failed to initialize SD!");
        while (1) delay(10);
    }

    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) {
        lcd.clear();
        lcd.print("Couldn't find FAT16/FAT32 partition!");
        while (1) delay(10);
    }

    uint32_t block_count = volume.blocksPerCluster()*volume.clusterCount();
    usb_msc.setCapacity(block_count, 512); // Set disk size, SD block size is always 512
    usb_msc.setUnitReady(true); // msc is ready for read/write
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  (void) bufsize;
  return card.readBlock(lba, (uint8_t*) buffer) ? 512 : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  (void) bufsize;
  return card.writeBlock(lba, buffer) ? 512 : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // nothing to do
}*/
