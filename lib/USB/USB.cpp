#include <USB.h>

SdFat card;

SdFile root;
SdFile file;

Adafruit_USBD_MSC usb_msc;

bool fs_changed;

void setupUSB() {
    // Set disk vendor id, product id and revision with string up to 8, 16, 4
    // characters respectively
    usb_msc.setID("Vendor", "Cuisinventory", "1.0");

    // Set read write callback
    usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

    // Still initialize MSC but tell usb stack that MSC is not ready to
    // read/write If we don't initialize, board will be enumerated as CDC only
    usb_msc.setUnitReady(false);
    usb_msc.begin();

    if (!card.begin(SD_PIN, SPI_HALF_SPEED)) {
        LOG("Failed to initialize SD!");
        while (1) delay(10);
    }

    // Size in blocks (512 bytes)
#if SD_FAT_VERSION >= 20000
    uint32_t block_count = card.card()->sectorCount();
#else
    uint32_t block_count = card.card()->cardSize();
#endif

    usb_msc.setCapacity(block_count,
                        512);    // Set disk size, SD block size is always 512
    usb_msc.setUnitReady(true);  // msc is ready for read/write
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
    bool rc;

#if SD_FAT_VERSION >= 20000
    rc = card.card()->readSectors(lba, (uint8_t*)buffer, bufsize / 512);
#else
    rc = card.card()->readBlocks(lba, (uint8_t*)buffer, bufsize / 512);
#endif

    return rc ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
    bool rc;

#if SD_FAT_VERSION >= 20000
    rc = card.card()->writeSectors(lba, buffer, bufsize / 512);
#else
    rc = card.card()->writeBlocks(lba, buffer, bufsize / 512);
#endif

    return rc ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and
// accepted by host). used to flush any pending cache.
void msc_flush_cb(void) {
#if SD_FAT_VERSION >= 20000
    card.card()->syncDevice();
#else
    card.card()->syncBlocks();
#endif

    // clear file system's cache to force refresh
    card.cacheClear();
}
