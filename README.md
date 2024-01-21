
![Cuisinventory](./Cuisinventory_Case_Design.gif)

# Cuisinventory

Cuisinventory serves as an grocery inventory management tool, simplifying the tracking of food consumption and detailed product specifics. This system is embodied in a station equipped with a barcode scanner and a weight scale for interactive use. By scanning the barcode of grocery items at the station, Cuisinventory promptly retrieves pertinent details such as product name, brand, allergens, and storage guidelines. The integrated weighing feature allows for the measurement of grocery weight, and when combined with the system's data on product quantity, Cuisinventory accurately calculates the remaining food quantity as a percentage. All inventory data is stored in a bespoke database and can be conveniently accessed and managed through a dedicated web application that communicates seamlessly with the station.

# User Manual

## System Overview

The Cuisinventory system is equipped with an LCD interface and three tactile buttons:

- GB1 (Green Button 1): Located above and to the left of the LCD screen.
- GB2 (Green Button 2): Positioned below and to the left of the LCD screen.
- RB (Red Button): Situated to the right of the LCD screen.

## Initial setup

### USB-Mode Configuration

1. Connect the system to your computer using a USB cable.
2. Press and hold RB to enter USB-Mode.
3. On your computer, locate and open the SD-Storage of the system.
4. Edit the *settings.json* file to input your Wi-Fi details:
   - "SSID": Your Wi-Fi network name.
   - "PASSWORD": Your Wi-Fi password.
5. Save the file and disconnect the USB cable.

### System Registration

1. Locate the system ID beneath the device.
2. Visit [ephraimsiegfried.github.io/cuisinventory-web](https://ephraimsiegfried.github.io/cuisinventory-web/).
3. Log in using your system ID to access and manage your kitchen inventory online.

## Daily Operations

### Adding new Products

1. Short press GB1 to initiate the addition process.
2. Position the product barcode facing downwards and scan it.
3. Place the product on the Cuisinventory scale.
4. Confirm to weigh by pressing GB1.
5. A success message indicates the product has been added successfully.

### Updating new products

1. Short press GB2 to start the update process.
2. Repeat the scanning and weighing process as described in the addition of new products.
3. A success message confirms the product weight has been updated.

### Removing Products

1. Short press RB to initiate removal.
2. Scan the product barcode.
3. A success message confirms the product has been removed.

### Inspecting Products on the System LCD

1. Long press GB1 to view products.
2. Navigate through products:
   - Next product: Short press GB1.
   - Previous product: Short press GB2.
3. Exit the view by long pressing RB.

### Inspecting Products on the Cuisinventory Website

1. Visit [ephraimsiegfried.github.io/cuisinventory-web](https://ephraimsiegfried.github.io/cuisinventory-web/) to view your entire inventory.
2. Products can be sorted by date or quantity for easier management.

### Note: Aborting

For any operation, you can abort and return to the main screen by long pressing RB.
