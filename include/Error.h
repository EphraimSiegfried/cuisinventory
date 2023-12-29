#pragma once

#include <Arduino.h>
#include <Constants.h>
#include <RTClib.h>
#include <SD.h>

class ErrorClass {
   public:
    void logError(String error);
};

extern ErrorClass Error;
