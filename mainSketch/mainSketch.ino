// Included Libraries
#include <AccelStepper.h>
#include "Adafruit_MAX31855.h"
#include <LiquidCrystal_I2C.h>

// Thermocouple Digital IO Pins
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

// Initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Display Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper Motor Digital IO Pins
const int dirPin = 2;
const int stepPin = 3;

// Define Motor Interface
#define motorInterfaceType 1

// Create Stepper Motor Instance
AccelStepper myStepper(motorInterface, stepPin, dirPin);

void setup() {
  // Setup Display
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Stepper Motor Setup
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(50);
  myStepper.setSpeed(200);

  setDisplay("Set Starting", "Height.");

}

void loop() {
  // put your main code here, to run repeatedly:

}


void setDisplay(char line1[], char line2[])
{
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}



















