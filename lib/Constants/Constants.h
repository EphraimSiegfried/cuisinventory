#pragma once

#include <Arduino.h>

// Main
const uint64_t IDLE_WAIT_BEFORE_SYNC = 300000;  // ms

// Barcode API
extern const char BARCODE_ENDPOINT[];
extern const char BARCODE_PATH [];
extern const char BARCODE_FIELDS[];
extern const char USER_AGENT [];

// Pythonanywhere server
extern const char PYTHONANYWHERE_ENDPOINT[];
extern const char PYTHONANYWHERE_PATH[];

// Device constants
#ifndef PIO_UNIT_TESTING
const String DEVICE_KEY = "th1s1sak3y";
#else
const String DEVICE_KEY = "th1s1satestk3y";
#endif

// SD
const int JSONSIZE = 1024;
const int STATEFILESIZE = 100;
const int SD_PIN = 10;
extern const char INTERNAL_FOLDER[];
extern const char STATE_FOLDER[];
extern const char DATA_FOLDER [];
extern const char SETTINGSFILE[];
extern const char ID_BAR_MAPPINGFILE[];
extern const char BAR_ID_MAPPINGFILE[];
extern const char STATEFILE[];

// JSON
const char UNIQUE_ID[] = "id";

// Clock
const uint32_t CLOCK_OFFSET = -17;  // offset of RTC in seconds

// Scale
const uint32_t STABILITY_THRESHOLD = 1;  // TODO: Change constant
const uint32_t SCALING = 400;            // TODO: Change constant
const uint32_t SAMPLE_AMOUNT = 10;       // TODO: Change constant

// Buttons
const uint8_t BTN_LIGHT_IDLE =
    10;  // brightness of buttons while idle, between 0 and 255
const uint8_t BTN_LIGHT_ACTIVE =
    50;  // brightness of buttons during click, between 0 and 255 LCD
const uint8_t LCD_CONTRAST = 5;
const uint8_t GREEN_BUTTON1_ADDRESS = 0x70;
const uint8_t GREEN_BUTTON2_ADDRESS = 0x6f;
const uint8_t RED_BUTTON_ADDRESS = 0x71;
const uint32_t LONG_PRESS_DURATION =
    2000;  // duration threshold of a long press
// Don't register button presses that have happened this many ms before we
// checked for input This is so that if btn1 is pressed while we're waiting for
// btn2, it doesn't get registered next time we check for btn1
const uint32_t PRESS_AGE_THRESHOLD = 3000;

// UART Barcode
const uint8_t RX_PIN = 5;
const uint8_t TX_PIN = 6;

#ifdef PIO_UNIT_TESTING
const String SSID = "iPhone";
const String PASSWORD = "12345678";
const String SAMPLE_PRODUCT_JSON =
    "{\"code\":\"3017620422003\",\"errors\":[],\"product\":{\"allergens\":\"en:"
    "milk,en:nuts,en:soybeans\",\"brands\":\"Ferrero\",\"conservation_"
    "conditions\":\"A conserver au sec et à l'abri de la chaleur. Ne pas "
    "mettre au réfrigérateur.\",\"customer_service\":\"FERRERO FRANCE "
    "COMMERCIALE - Service Consommateurs, CS 90058 - 76136 MONT SAINT AIGNAN "
    "Cedex\",\"generic_name\":\"Pâte à tartiner aux noisettes et au "
    "cacao\",\"image_url\":\"https://images.openfoodfacts.org/images/products/"
    "301/762/042/2003/front_en.550.400.jpg\",\"ingredients_text\":\"Sucre, "
    "huile de palme, NOISETTES 13%, cacao maigre 7,4%, LAIT écrémé en poudre "
    "6,6%, LACTOSERUM en poudre, émulsifiants: lécithines [SOJA), vanilline. "
    "Sans "
    "gluten.\",\"product_name\":\"Nutella\",\"product_quantity\":\"400\"},"
    "\"result\":{\"id\":\"product_found\",\"lc_name\":\"Product "
    "found\",\"name\":\"Product "
    "found\"},\"status\":\"success\",\"warnings\":[]}";
#endif
