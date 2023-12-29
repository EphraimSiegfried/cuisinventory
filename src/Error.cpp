#include <Error.h>

RTC_PCF8523 rtc;
File logFile;

void Error::logError(String error) {
#ifdef DEBUG
    Serial.println("Error: " + error);
#endif
    DateTime now = DateTime(rtc.now().unixtime() - CLOCK_OFFSET);
    logFile = SD.open(LOGFILE, FILE_WRITE);
    if (logFile) {
        logFile.println(now.timestamp() + " Error: " + error);
        logFile.close();
    }
}
