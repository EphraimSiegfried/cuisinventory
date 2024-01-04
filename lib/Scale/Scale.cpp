#include <Scale.h>

Adafruit_NAU7802 nau;
bool initScale() {
    if (!nau.begin()) {
        LOG("Failed to find NAU7802");
        return false;
    }
    nau.setLDO(NAU7802_3V9);
    nau.setGain(NAU7802_GAIN_128);
    nau.setRate(NAU7802_RATE_10SPS);
    for (uint8_t i = 0; i < 10; i++) {
        while (!nau.available()) delay(1);
        nau.read();
    }

    bool calibrated = false;
    for (int i = 0; i < 10 && !nau.calibrate(NAU7802_CALMOD_INTERNAL); i++) {
        delay(500);
        if (nau.calibrate(NAU7802_CALMOD_INTERNAL)) {
            calibrated = true;
        }
    }
    if (!calibrated) {
        LOG("Failed to calibrate internal offset");
        return false;
    }

    calibrated = false;
    for (int i = 0; i < 10 && !nau.calibrate(NAU7802_CALMOD_OFFSET); i++) {
        delay(500);
        if (nau.calibrate(NAU7802_CALMOD_OFFSET)) {
            calibrated = true;
        }
    }
    if (!calibrated) {
        LOG("Failed to calibrate system offset");
        return false;
    }
    LOG("Calibrated system offset");
    return true;
}
