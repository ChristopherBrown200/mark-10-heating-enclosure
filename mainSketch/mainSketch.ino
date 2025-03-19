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
const double maxTemp = 150.0; //TODO: Find Actual Max


// Display Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper Motor Digital IO Pins
const int dirPin = 2;
const int stepPin = 3;

// Define Motor Interface
#define motorInterfaceType 1

// Create Stepper Motor Instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

// Motor Varibles
const double MaxSpeed = 70.0;
double Speed = 10.0; // TODO: Set as min speed
bool returnEnabled = false;
unsigned long timeA = 0;
unsigned long timeB = 0;

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
int prev_CLK_state;

// Relay Pin
#define RELAY_PIN 99


// Boot squence and Pretest Setup ======================================================
void setup() {
  Serial.begin(9600);

  while (!Serial) delay(1);

  // Relay Setup
  pinMode(RELAY_PIN, OUTPUT);

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
  prev_CLK_state = digitalRead(CLK_PIN);

  // Setup Thermocouple
  if (!thermocouple.begin()){
    Serial.println("Thermocouple Error");
    while (1) delay(10);
  }
  actualTemp = thermocouple.readCelsius();

  // Set the starting height
  setDisplay("Set Starting", "Height.");
  while (digitalRead(SW_PIN) == 1){
    CLK_state = digitalRead(CLK_PIN);
    if (CLK_state != prev_CLK_state){
      if (digitalRead(DT_PIN) == HIGH){
        myStepper.move(200);
        direction = DIRECTION_CCW;
      }
      else{
        myStepper.move(-200);
        direction = DIRECTION_CW;
      }
      myStepper.run();
      prev_CLK_state = CLK_state;
    }
  }

  // Set the Expansion Rate
  setDisplay("Set Expansion", "Speed:");
  while (digitalRead(SW_PIN) == 1){
    CLK_state = digitalRead(CLK_PIN);
    if (CLK_state != prev_CLK_state){
      if (digitalRead(DT_PIN) == HIGH){
        if (Speed < MaxSpeed){
          Speed += 0.1;
        }
        direction = DIRECTION_CCW;
      }
      else{
        if (Speed > 0){
          Speed -= 0.1;
        }
        direction = DIRECTION_CW;
      }
      prev_CLK_state = CLK_state;

      lcd.setCursor(1, 6);
      lcd.print("          ");
      lcd.setCursor(1, 6);
      lcd.print(Speed);
      lcd.print(" in/min");
    }
  }

  setDisplay("Return Home?", "N");
  while (digitalRead(SW_PIN) == 1){
    CLK_state = digitalRead(CLK_PIN);
    if (CLK_state != prev_CLK_state){
      returnEnabled = !returnEnabled;
      if (returnEnabled){
        setDisplay("Return Home?", "Y");
      }
      else {
        setDisplay("Return Home?", "N");
      }
    }
  }

  // Prompt the user to close the enclosure
  setDisplay("Please ensure", "door is closed.");
  while (digitalRead(SW_PIN) == 1) delay(1);

  // Get the desired tempeture
  setDisplay("Tempeture", "");
  while (digitalRead(SW_PIN) == 1){
    CLK_state = digitalRead(CLK_PIN);
    if (CLK_state != prev_CLK_state){
      if (digitalRead(DT_PIN) == HIGH){
        if (goalTemp < maxTemp){
          goalTemp += 1.0;
        }
        direction = DIRECTION_CCW;
      }
      else {
        if (goalTemp > 0){
          goalTemp -= 1.0;
        }
        direction = DIRECTION_CW;
      }
      prev_CLK_state = CLK_state;

      lcd.setCursor(1, 0);
      lcd.print(goalTemp);
      lcd.print("C");
    }
  }

  // Wait for enclosure to reach tempeture
  setDisplay("Heating", "Chamber");
  digitalWrite(RELAY_PIN, HIGH);
  do{
    actualTemp = thermocouple.readCelsius();
  } while (actualTemp < goalTemp);

  setDisplay("Ready Press to", "Start");
  while (digitalRead(SW_PIN) == 1) maintainTemp();
}

// Need for reset after test
void(* resetFunc) (void) = 0;

void loop() {
  if (digitalRead(SW_PIN) == 0){
    digitalWrite(RELAY_PIN, LOW);
    if (returnEnabled){
      returnHome();
    }
    resetFunc();
  }
  maintainTemp();

  timeA = millis();
}

void maintainTemp(){
  actualTemp = thermocouple.readCelsius();

  if (actualTemp >= goalTemp + 2.5){
    digitalWrite(RELAY_PIN, HIGH);
  }
  else if (actualTemp < goalTemp - 1.0){
    digitalWrite(RELAY_PIN, LOW);
  }
}

void returnHome(){
  // TODO
}

void setDisplay(char line1[], char line2[])
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}



















