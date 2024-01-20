#include "BarReader.h"

SoftwareSerial softSerial(TX_PIN, RX_PIN);
DE2120 scanner;

bool initBarReader() {
    if (!scanner.begin(softSerial)) {
        LOG("Couldn't initialize Barcode Reader");
        return false;
    }
    return true;
}

bool readBar(String& scannedCode) {
    char scanBuffer[40];
    if (scanner.readBarcode(scanBuffer, 40)) {
        scannedCode = String(scanBuffer);
        return true;
    }
    return false;
}
