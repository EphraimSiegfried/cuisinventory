#pragma once
#include <Arduino.h>

// Barcode API
const String BARCODE_ENDPOINT = "world.openfoodfacts.org";
const String BARCODE_PATH = "/api/v3/product/";
const String BARCODE_FIELDS =
    "product_name,generic_name,allergens,conservation_conditions,nutriscore_"
    "grade,ingredients_text,customer_service";
const String USER_AGENT = "Cuisinventory/1.0 alexander.lutsch@stud.unibas.ch";

// Tasks
const uint64_t T_INPUT_INTERVAL =
    100;  // interval between calls to checkInput in ms

// SD
const int JSONSIZE = 1024;
const int STATEFILESIZE = 1024;
const int SD_PIN = 10;
const String KEY_BAR_MAPPINGFILE = "/.intern/keybar";
const String BAR_KEYS_MAPPINGFILE = "/.intern/barkey";
const String STATEFILE = "/.state/state";
const String LOGFILE = "/.state/logs";

// Clock
const uint32_t CLOCK_OFFSET = -17;  // offset of RTC in seconds

// Buttons
const uint8_t BTN_LIGHT_IDLE =
    10;  // brightness of buttons while idle, between 0 and 255
const uint8_t BTN_LIGHT_ACTIVE =
    50;  // brightness of buttons during click, between 0 and 255 LCD
const uint8_t LCD_CONTRAST = 5;
