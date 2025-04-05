/*
Project: Controlled Heat Enclosure for the Mark-10 F1505-IM
Client: Dr. Alireza Sarvestani
Author: Christopher Brown
Date: April 7, 2025
*/

// Included Libraries
#include <AccelStepper.h>
#include "Adafruit_MAX31855.h"
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>

// Thermocouple Digital IO Pins
#define MAXDO   7
#define MAXCS   6
#define MAXCLK  5

// Initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

double actualTemp = 0.0;
int goalTemp = 70;
const double maxTemp = 34.0;


// Display Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper Motor Digital IO Pins
const int dirPin = 9;
const int stepPin =8;

// Define Motor Interface
#define motorInterfaceType 1

// Create Stepper Motor Instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

// Motor Varibles
const double MaxSpeed = 10.0; // NOTE: Due to mechanical problems, this value could not be determined.
double speedInMin = 10.0; 
double speedInSec = 0.0;
bool returnEnabled = false;
long stepsRotated = 0;
const double inPerStep = 0.00026667;
double disTraveled = 0.0;
unsigned long startTime = 0;

// Define encoder Pins
#define CLK 22
#define DT 24
#define SW 26

// Encoder direction definitions
#define DIRECTION_CW  0
#define DIRECTION_CCW 1

// Encoder Varibles
int direction = DIRECTION_CW;
int CLKstate;
int prevCLKstate;

// Relay Pin
#define RELAY 52


// Boot squence and Pretest Setup ======================================================
void setup() {
  Serial.begin(9600);

  while (!Serial) delay(1);

  Serial.println("Started");

  // Relay Setup
  pinMode(RELAY, OUTPUT);

  // Setup Display
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Stepper Motor Setup
  myStepper.setMaxSpeed(10000);
  myStepper.setAcceleration(10000);
  myStepper.setSpeed(200);
  // myStepper.moveTo(200)

  // Setup Encoder
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  prevCLKstate = digitalRead(CLK);

  // Setup Thermocouple
  if (!thermocouple.begin()){
    Serial.println("Thermocouple Error");
    while (1) delay(10);
  }
  actualTemp = thermocouple.readCelsius();

  // Set the starting height
  setDisplay("Set Starting", "Height.");
  bool moving = false;
  bool up = false;
  while (digitalRead(SW) == 1){
    CLKstate = digitalRead(CLK);
    if (CLKstate != prevCLKstate && CLKstate == 1){
      if (digitalRead(DT) == CLKstate){
        if (myStepper.isRunning()){
          if (!up){
            myStepper.stop();
          }
        }
        else{
          up = true;
        }
        Serial.println("Moved UP");
        myStepper.moveTo(myStepper.targetPosition() - 200);
      }
      else{
        if (myStepper.isRunning()){
          if (up){
            myStepper.stop();
          }
        }
        else{
          up = false;
        }
        Serial.println("Moved Down");
        myStepper.moveTo(myStepper.targetPosition() + 200);
      }
    }
    prevCLKstate = CLKstate;
    myStepper.run();
    delay(1);
  }
  Serial.println("Starting Height Set.");
  delay(500);

  // Set the Expansion Rate
  setDisplay("Set Expansion", "Speed:");
  lcd.setCursor(1, 6);
  lcd.print("          ");
  lcd.setCursor(1, 6);
  lcd.print(speedInMin);
  lcd.print(" in/min");
  Serial.print(speedInMin);
  Serial.println(" in/min");

  while (digitalRead(SW) == 1){
    CLKstate = digitalRead(CLK);
    if (CLKstate != prevCLKstate && CLKstate == 1){
      if (digitalRead(DT) == CLKstate){
        if (speedInMin < MaxSpeed){
          speedInMin += 0.1;
        }
      }
      else{
        if (speedInMin > 0.1){
          speedInMin -= 0.1;
        }
      }

      lcd.setCursor(1, 6);
      lcd.print("          ");
      lcd.setCursor(1, 6);
      lcd.print(speedInMin);
      lcd.print(" in/min");
      Serial.print(speedInMin);
      Serial.println(" in/min");
    }
    prevCLKstate = CLKstate;
    delay(1);
  }

  Serial.print("Expansion Rate Set: ");
  Serial.print(speedInMin);
  Serial.println(" in/min");
  delay(500);

  // //Converts speed tp in/sec
  speedInSec = speedInMin / 60.0;

  setDisplay("Return Home?", "No");
  while (digitalRead(SW) == 1){
    CLKstate = digitalRead(CLK);
    if (CLKstate != prevCLKstate && CLKstate == 1){
      returnEnabled = !returnEnabled;
      if (returnEnabled){
        setDisplay("Return Home?", "Yes");
      }
      else {
        setDisplay("Return Home?", "No");
      }
    }
    prevCLKstate = CLKstate;
    delay(1);
  }
  Serial.print("Return Home: ");
  Serial.println(returnEnabled);
  delay(500);

  // // Prompt the user to close the enclosure
  setDisplay("Please ensure", "door is closed.");
  while (digitalRead(SW) == 1) delay(1);

  Serial.println("Door Close Confirmed.");
  delay(500);

  // // Get the desired tempeture
  setDisplay("Tempeture", "");
  while (digitalRead(SW) == 1){
    CLKstate = digitalRead(CLK);
    if (CLKstate != prevCLKstate && CLKstate == 1){
      if (digitalRead(DT) == CLKstate){
        if (goalTemp < maxTemp){
          goalTemp += 1;
        }
        direction = DIRECTION_CCW;
      }
      else {
        if (goalTemp > 21){
          goalTemp -= 1;
        }
        direction = DIRECTION_CW;
      }

      lcd.setCursor(1, 0);
      lcd.print(goalTemp);
      lcd.print("C");

      Serial.print((int)goalTemp);
      Serial.println("C");
    }
    prevCLKstate = CLKstate;
  }
  Serial.print("Tempeture Set: ");
  Serial.print(goalTemp);
  Serial.println("C");
  delay(500);


  // Wait for enclosure to reach tempeture
  setDisplay("Heating", "Chamber");
  digitalWrite(RELAY, HIGH);
  do{
    actualTemp = thermocouple.readCelsius();

    if (isnan(actualTemp)){
      setDisplay("Thermocouple", "Error!");
      while(1) delay(1000);
    }

    Serial.print("Heating: ");
    Serial.print(actualTemp);
    Serial.println("C");
  } while (actualTemp < goalTemp);

  setDisplay("Ready, Press to", "Start");
  // while (digitalRead(SW) == 1) maintainTemp();
  startTime = millis();
  Serial.print("Test Started.");
  delay(500);
}

// Need for reset after test
void(* resetFunc) (void) = 0;

void loop() {
  if (digitalRead(SW) == 0){
    digitalWrite(RELAY, LOW);
    Serial.println("Test Stopped.");

    if (returnEnabled){
      returnHome();
    }

    resetFunc();
  }
  maintainTemp();

  long timePassed = millis() - startTime;
  maintainExpansion(timePassed);
  delay(1);
}

void maintainTemp(){
  actualTemp = thermocouple.readCelsius();

  // if (actualTemp >= goalTemp + 2.5){
  //   digitalWrite(RELAY, HIGH);
  //   Serial.println("Heating turned on.");
  // }
  // else if (actualTemp < goalTemp - 1.0){
  //   digitalWrite(RELAY, LOW);
  //   Serial.println("Heating turned off.");
  // }
}

void maintainExpansion(long msPassed){
  double secPassed = msPassed / 1000.0;
  double correctDis = speedInSec * secPassed;
  if (correctDis > disTraveled){
    myStepper.moveTo(myStepper.targetPosition() - 200); //TODO: Amount
    disTraveled += (200 * inPerStep); //TODO: FindAmount
    stepsRotated += 200; //TODO: Match Amount
  }
  myStepper.run();
}

void returnHome(){
  setDisplay("Returning", "Home");
  myStepper.move(-stepsRotated); //TODO: Direction
  myStepper.run();
  while(myStepper.distanceToGo() != 0) delay(1);
}

void setDisplay(char line1[], char line2[])
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);

  Serial.println();
  Serial.println("Screen:");
  Serial.println("----------------");
  Serial.println(line1);
  Serial.println(line2);
  Serial.println("----------------");
  Serial.println();
}
