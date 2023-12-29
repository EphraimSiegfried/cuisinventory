#pragma once

#include <Arduino.h>
#include <Constants.h>
#include <SD.h>
#include <RTClib.h>


class Error {
    public:
     void logError(String error);
};
