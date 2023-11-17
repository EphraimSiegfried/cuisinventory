#include <SparkFun_Qwiic_Button.h>
#include <Wire.h>

#include <SerLCD.h> 
QwiicButton greenButton1;
QwiicButton greenButton2;
QwiicButton redButton;
SerLCD lcd; 

void setup() {
  Wire.begin();
  lcd.begin(Wire);
  lcd.setBacklight(255, 255, 255); //Set backlight to bright white
  lcd.setContrast(5); //Set contrast. Lower to 0 for higher contrast.
  lcd.clear(); 

  if (greenButton1.begin(0x70) == false) {
    Serial.println("Green Device did not acknowledge! Freezing.");
    while (1);
  }
  if (greenButton2.begin(0x6f) == false) {
    Serial.println("Green Device did not acknowledge! Freezing.");
    while (1);
  }
  if (redButton.begin(0x71) == false) {
    Serial.println("Red Device did not acknowledge! Freezing.");
    while (1);
  }
  Serial.println("Buttons acknowledged.");

}

void loop() {
  String buttonAd = "Nothing";

  if (greenButton1.isPressed()) {
    buttonAd = "Green 0x70";
    lcd.setBacklight(0, 255, 0); // Green
  } else if (greenButton2.isPressed()) {
    buttonAd = "Green 0x71";
    lcd.setBacklight(0, 0, 255); // bright blue
  } else if (redButton.isPressed()) {
    buttonAd = "Red 0x70";
    lcd.setBacklight(255, 0, 0); // Red
  }
  lcd.print(buttonAd + " pressed");
  while (buttonAd.isPressed())
    delay(30);  //wait for user to stop pressing
  lcd.clear();
}
