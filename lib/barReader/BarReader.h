#include <SparkFun_DE2120_Arduino_Library.h>
#include <SoftwareSerial.h>
#include "Constants.h"
#include "Debug.h"


bool initBarReader();
bool readBar(String& scannedCode);
extern SoftwareSerial softSerial;
extern DE2120 scanner;