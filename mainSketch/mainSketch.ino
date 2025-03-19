// Included Libraries
#include <AccelStepper.h>
#include "Adafruit_MAX31855.h"
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>

// Thermocouple Digital IO Pins
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

// Initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

double actualTemp = 0.0;
double goalTemp = 70.0;
const double maxTempeture = 150.0; //TODO: Find Actual Max


// Display Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper Motor Digital IO Pins
const int dirPin = 2;
const int stepPin = 3;

// Define Motor Interface
#define motorInterfaceType 1

// Create Stepper Motor Instance
AccelStepper myStepper(motorInterface, stepPin, dirPin);

// Define encoder Pins
#define CLK_PIN 27
#define DT_PIN  26
#define SW_PIN  25

// Encoder direction definitions
#define DIRECTION_CW  0
#define DIRECTION_CCW 1

// Encoder Varibles
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state


// Boot squence and Pretest Setup ======================================================
void setup() {
  Serial.begin(9600);

  while (!Serial) delay(1);

  // Setup Display
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Stepper Motor Setup
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(50);
  myStepper.setSpeed(200);

  // Setup Encoder
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT);
  prev_CLK_State = digitalRead(CLK_PIN);

  // Setup Thermocouple
  if (!thermocouple.begin()){
    Serial.println("Thermocouple Error")
    while (1) delay(10);
  }
  actualTemp = thermocouple.readInternal()

  // Set the starting height
  setDisplay("Set Starting", "Height.");
  while (digitalRead(SW_PIN) == 1){
    CLK_state = digitalRead(CLK_PIN);
    if (CLK_state !+ prev_CLK_State){
      if (digitalRead(DT_PIN) == HIGH){
        // Move UP
        direction = DIRECTION_CCW;
      }
      else{
        // Move Down
        direction = DIRECTION_CW;
      }
      prev_CLK_state = CLK_state;
    }
  }

  // Prompt the user to close the enclosure
  setDisplay("Please ensure", "door is closed.")
  while (digitalRead(SW_PIN) == 1) delay(1);

  // Get the desired tempeture
  setDisplay("Tempeture", "")
  while (digitalRead(SW_PIN) == 1){
    CLK_state = digitalRead(CLK_PIN);
    if (CLK_state != prev_CLK_State){
      if (digitalRead(DT_PIN) == HIGH){
        goalTemp += 1.0;
        direction = DIRECTION_CCW;
      }
      else{
        goalTemp -= 1.0;
        direction = DIRECTION_CW;
      }
      prev_CLK_state = CLK_state;
    }

    // TODO: Display goalTemp
  }

  // Wait for enclosure to reach tempeture
  setDisplay("Heating", "Chamber");
  do{
    actualTemp = thermocouple.readInternal();
  } while (actualTemp < goalTemp);
}

void loop() {
  // put your main code here, to run repeatedly:

}


void setDisplay(char line1[], char line2[])
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}



















