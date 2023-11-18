#include <SPI.h>
#include <Wire.h>
#include <SerLCD.h> 
#include <WiFiNINA.h>

#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

SerLCD lcd;

void setup() {
    Wire.begin();
    lcd.begin(Wire);
    Wire.setClock(400000); // set I2C SCL to High Speed Mode of 400kHz

    lcd.setFastBacklight(0xFFFFFF); // set backlight to bright white
    lcd.setContrast(5); // Set contrast. Lower to 0 for higher contrast.
    lcd.clear(); 

    lcd.print("WiFi Scanning test");

    // Set up the pins!
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

    // check for the WiFi module:
    while (WiFi.status() == WL_NO_MODULE) {
        lcd.clear();
        lcd.print("Communication with WiFi module failed!");
        // don't continue
        delay(1000);
    }

    String fv = WiFi.firmwareVersion();
    if (fv < "1.0.0") {
        lcd.clear();
        lcd.print("Please upgrade the firmware");
        while (1) delay(10);
    }
    lcd.print("\nFirmware OK");

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    lcd.clear();
    lcd.print("MAC: ");
    printMacAddress(mac);
    delay(5000);
}

void loop() {
    // scan for existing networks:
    lcd.clear();
    lcd.print("Scanning available networks...");
    listNetworks();
    delay(10000);
}

void listNetworks() {
    // scan for nearby networks:
    lcd.clear();
    lcd.print("** Scan Networks **\n");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1) {
        lcd.print("Couldn't get a wifi connection");
        while (true);
    }

    // print the list of networks seen:
    lcd.print("available networks: ");
    lcd.print(numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        lcd.print(thisNet);
        lcd.print(") ");
        lcd.print(WiFi.SSID(thisNet));
        lcd.print("\tSignal: ");
        lcd.print(WiFi.RSSI(thisNet));
        lcd.print(" dBm");
        lcd.print("\tEncryption: ");
        printEncryptionType(WiFi.encryptionType(thisNet));
    }
}

void printEncryptionType(int thisType) {
    // read the encryption type and print out the name:
    switch (thisType) {
        case ENC_TYPE_WEP:
            lcd.print("WEP\n");
            break;
        case ENC_TYPE_TKIP:
            lcd.print("WPA\n");
            break;
        case ENC_TYPE_CCMP:
            lcd.print("WPA2\n");
            break;
        case ENC_TYPE_NONE:
            lcd.print("None\n");
            break;
        case ENC_TYPE_AUTO:
            lcd.print("Auto\n");
            break;
        case ENC_TYPE_UNKNOWN:
        default:
            lcd.print("Unknown\n");
            break;
    }
}


void printMacAddress(byte mac[]) {
    for (int i = 5; i >= 0; i--) {
        if (mac[i] < 16) {
            lcd.print("0");
        }
        lcd.print(mac[i], HEX);
        if (i > 0) {
            lcd.print(":");
        }
    }
    lcd.print("\n");
}
