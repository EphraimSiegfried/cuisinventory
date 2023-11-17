#include <SparkFun_Qwiic_Button.h>
#include <Wire.h>

#include <SerLCD.h> 
QwiicButton button;
SerLCD lcd; 

void setup() {
  Wire.begin();
  lcd.begin(Wire);
  lcd.setBacklight(255, 255, 255); //Set backlight to bright white
  lcd.setContrast(5); //Set contrast. Lower to 0 for higher contrast.
  lcd.clear(); 

  if (button.begin() == false) {
    Serial.println("Device did not acknowledge! Freezing.");
    while (1);
  }
  Serial.println("Button acknowledged.");

}

void loop() {
  // put your main code here, to run repeatedly:
  if (button.isPressed() == true) {
    Serial.println("The button is pressed!");
    lcd.setCursor(0, 1);
    lcd.print("The button is pressed");
    while (button.isPressed() == true)
      delay(10);  //wait for user to stop pressing
    lcd.clear();
  }

}
