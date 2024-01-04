#include <USB.h>

SdFat card;
Adafruit_USBD_MSC usb_msc;

void setupUSB() {
    // Set disk vendor id, product id and revision with string up to 8, 16, 4
    // characters respectively usb_msc.setID("Vendor", "Cuisinventory", "1.0");

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

    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or
    // FAT32
    if (!card.vol()->init(card.card())) {
        LOG("Couldn't find FAT16/FAT32 partition!");
        while (1) delay(10);
    }

    uint32_t block_count = card.vol()->sectorsPerCluster() * card.vol()->clusterCount();
    usb_msc.setCapacity(block_count,
                        512);    // Set disk size, SD block size is always 512
    usb_msc.setUnitReady(true);  // msc is ready for read/write
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
    (void)bufsize;
    return card.card()->readBlock(lba, (uint8_t*)buffer) ? 512 : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
    (void)bufsize;
    return card.card()->writeBlock(lba, buffer) ? 512 : -1;
}

// Callback invoked when WRITE10 command is completed (status received and
// accepted by host).
// used to flush any pending cache.
void msc_flush_cb(void) {
    // nothing to do
}
