#pragma once

#include <Arduino.h>
#include <Constants.h>
#include <RTClib.h>
#include <SD.h>

class Error {
   public:
    void logError(String error);
};
