#include <Adafruit_NAU7802.h>
#include <Constants.h>
#include <Debug.h>
bool weightIsSettled(int32_t &weight, unsigned int &stableReadingsCount,
                     unsigned int &unstableReadingsCount);
bool initScale();
