/*
 * File: main.cpp
 * --------------
 * Scoooooooooooooooooooooooooooooooooooooooooott Blankenberg, Perry, Obergruppenführer Steppenheimer, MerickatrΩnics Blankenberg
 * ME210
 * 2/5/2019
 * Final
 *
 * Description:
 * 	Implements Master FSM
 */

/*---------------------------- Function Definitions --------------------------*/

#include <Arduino.h>
#include <Metro.h>

//Global Functions
void checkEndGame();

// FSM Functions
void evaluateStates();
void handleInitWest();
void handleToArmoury();
void handleReload();
void handleHuntSouth();
void handleHuntNorth();
void handleDone();

// Motor Functions
void evaluateMotors();
void motorsNSNorth();
void motorsNSStop();
void motorsNSSouth();
void motorsEWWest();
void motorsEWStop();
void motorsEWEast();

// IO Functions
void bumperPoll();
void sensorISR();
void cannonSpool();
void cannonCooldown();
void cannonFire();

/*--------------------------------- Constants --------------------------------*/

#define TIME_MATCH           (((60 * 2) + 10) * 1E3) // Match duration (ms)
#define TIME_RELOAD          (5 * 1E3)               // Reload time (ms)

#define PIN_CANNON_SPOOL     19
#define PIN_CANNON_LAUNCH    18
#define PIN_PHOTOTRANSISTOR  12

#define PIN_NORTH_BUMPER     5
#define PIN_NORTH_MOTOR_1    4
#define PIN_NORTH_MOTOR_2    3

#define PIN_SOUTH_BUMPER     6
#define PIN_SOUTH_MOTOR_1    10
#define PIN_SOUTH_MOTOR_2    9

#define PIN_EAST_BUMPER      8
#define PIN_EAST_MOTOR_1     17
#define PIN_EAST_MOTOR_2     16

#define PIN_WEST_BUMPER      7
#define PIN_WEST_MOTOR_1     22
#define PIN_WEST_MOTOR_2     23

typedef enum {
  STATE_IDLE, STATE_INIT_WEST, STATE_ARMOURY_TO, STATE_ARMOURY_RELOAD, STATE_HUNTING_SOUTH, STATE_HUNTING_NORTH, STATE_DONE
} States_t;

typedef enum {
  MOTOR_STOPPED, MOTOR_HUG_WEST_GONORTH, MOTOR_HUG_WEST_GOSOUTH, MOTOR_HUG_SOUTH_GOWEST
} MotorMode_t;

/*------------------------------ Global Variables ----------------------------*/

// Timing for staying out of armoury
uint16_t totalTransferTime  = 0;
uint16_t armouryEscapeTime  = 0;
float armouryEscapeFraction = 0.2; // Percentage of area of armoury roughly, dont forget safety margin, actual percentage is

// Set by FSM, should be false in armoury, unloaded, etc.
bool fireEnabled = false;
// Set by ISR after fire command delivered
bool hasFired    = false;

// Current movement mode
MotorMode_t currentMotorMovement = MOTOR_STOPPED;

// Current state
States_t currentState = STATE_IDLE;
bool justTransitioned = false;

// True when pressed down
bool northBumper = false;
bool southBumper = false;
bool eastBumper  = false;
bool westBumper  = false;

/*----------------------------------- Main -----------------------------------*/

void setup() {

  Serial.begin(9600);

  pinMode(PIN_CANNON_SPOOL, OUTPUT);
  digitalWrite(PIN_CANNON_SPOOL, LOW);

  pinMode(PIN_CANNON_LAUNCH, OUTPUT);
  digitalWrite(PIN_CANNON_LAUNCH, LOW);

  //Phototransistor pins
  pinMode(PIN_PHOTOTRANSISTOR, INPUT_PULLDOWN);

  // Need to update

  //Drive train pins
  pinMode(PIN_NORTH_MOTOR_1, OUTPUT);
  pinMode(PIN_NORTH_MOTOR_2, OUTPUT);
  digitalWrite(PIN_NORTH_MOTOR_1, LOW);
  digitalWrite(PIN_NORTH_MOTOR_2, LOW);

  pinMode(PIN_SOUTH_MOTOR_1, OUTPUT);
  pinMode(PIN_SOUTH_MOTOR_2, OUTPUT);
  digitalWrite(PIN_SOUTH_MOTOR_1, LOW);
  digitalWrite(PIN_SOUTH_MOTOR_2, LOW);

  pinMode(PIN_EAST_MOTOR_1, OUTPUT);
  pinMode(PIN_EAST_MOTOR_2, OUTPUT);
  digitalWrite(PIN_EAST_MOTOR_1, LOW);
  digitalWrite(PIN_EAST_MOTOR_2, LOW);

  pinMode(PIN_WEST_MOTOR_1, OUTPUT);
  pinMode(PIN_WEST_MOTOR_2, OUTPUT);
  digitalWrite(PIN_WEST_MOTOR_1, LOW);
  digitalWrite(PIN_WEST_MOTOR_2, LOW);

  //switch pins for drive train
  pinMode(PIN_SOUTH_BUMPER, INPUT_PULLUP);
  pinMode(PIN_NORTH_BUMPER, INPUT_PULLUP);
  pinMode(PIN_EAST_BUMPER, INPUT_PULLUP);
  pinMode(PIN_WEST_BUMPER, INPUT_PULLUP);

  //debug LED
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop() {
  bumperPoll();
  evaluateStates();
  evaluateMotors();
}


/*-------------------------------- FSM Functions -----------------------------*/

void evaluateStates() {
  switch(currentState) {
    case STATE_IDLE:
      currentState = STATE_INIT_WEST;
      justTransitioned = true;
      break;
    case STATE_INIT_WEST:
      handleInitWest();
      break;
    case STATE_ARMOURY_TO:
      handleToArmoury();
      break;
    case STATE_ARMOURY_RELOAD:
      handleReload();
      break;
    case STATE_HUNTING_NORTH:
      handleHuntNorth();
      break;
    case STATE_HUNTING_SOUTH:
      handleHuntSouth();
      break;
    case STATE_DONE:
      handleDone();
      break;
  }
}

void handleInitWest() {
  // Entering state
  if(justTransitioned) {
    justTransitioned = false;
  }

  // State content
  currentMotorMovement = MOTOR_HUG_SOUTH_GOWEST;

  // Leaving state
  if(westBumper) {
    justTransitioned = true;
    currentState = STATE_ARMOURY_TO;
  }

  checkEndGame();

}

void handleToArmoury() {
  // Used to calculate how long it takes to cross over
  static bool hasStartedCalculatedTransfer  = false;
  static bool hasFinishedCalculatedTransfer = false;
  static uint16_t transferStartTime         = 0;
  // Entering state
  if(justTransitioned) {
    justTransitioned = false;
    // Calculates transfer start time, once
    if(!hasStartedCalculatedTransfer) {
      hasStartedCalculatedTransfer = true;
      transferStartTime = millis();
    }
  }
  // State content
  currentMotorMovement = MOTOR_HUG_WEST_GONORTH;
  // Leaving state
  if(northBumper) {
    currentState = STATE_ARMOURY_RELOAD;
    justTransitioned = true;
    // Calculates time on exit, once
    if(!hasFinishedCalculatedTransfer) {
      hasFinishedCalculatedTransfer = true;
      totalTransferTime = millis() - transferStartTime;
      armouryEscapeTime = (uint16_t) ((float) totalTransferTime * armouryEscapeFraction);
    }
  }

  // End of game
  checkEndGame();
}

void handleReload() {
  static uint16_t startReloadTime;
  // Entering state
  if(justTransitioned) {
    justTransitioned = false;
    startReloadTime  = millis();
  }
  // State content
  uint16_t currentRunTime = millis() - startReloadTime;
  currentMotorMovement = MOTOR_STOPPED;
  // State transition
  if(currentRunTime > TIME_RELOAD) {
    justTransitioned = true;
    hasFired         = false;
    currentState     = STATE_HUNTING_SOUTH;
  }

  // End of game
  checkEndGame();
}

void handleHuntSouth() {
  static uint16_t startSouthHuntTime = 0;
  // Entering state
  if(justTransitioned) {
    justTransitioned = false;
    startSouthHuntTime = millis();
  }
  // State content
  uint16_t currentTime = millis() - startSouthHuntTime;
  if(currentTime > armouryEscapeTime) { // Keeps us out of the armoury
    fireEnabled = true;
  } else {
    fireEnabled = false;
  }
  // State transition
  if(southBumper) {
    currentState = STATE_HUNTING_NORTH;
    justTransitioned = true;
  } else if(hasFired) {
    hasFired         = false;
    justTransitioned = true;
    currentState     = STATE_ARMOURY_TO;
  }

  // End of game
  checkEndGame();
}

void handleHuntNorth() {
  static uint16_t startNorthHuntTime = 0;
  // Entering state
  if(justTransitioned) {
    justTransitioned = false;
    startNorthHuntTime = millis();
  }
  // State content
  uint16_t currentTime = millis() - startNorthHuntTime;
  if(currentTime < (totalTransferTime - armouryEscapeTime)) { // Approaching Armoury
    fireEnabled = true;
  } else {
    fireEnabled = false;
  }
  // State transition
  if(northBumper) {
    currentState = STATE_HUNTING_SOUTH;
    justTransitioned = true;
  } else if(hasFired) {
    hasFired         = false;
    justTransitioned = true;
    currentState     = STATE_ARMOURY_TO;
  }

  // End of game
  checkEndGame();
}

void handleDone() {
  // Enters state
  if(justTransitioned) {
    cannonCooldown();
  }
  // State content
  currentMotorMovement = MOTOR_STOPPED;
  // You can never leave :)
}

void checkEndGame() {
  if(millis() > TIME_MATCH) {
    justTransitioned = true;
    currentState = STATE_DONE;
  }
}

/*------------------------------- Motor Functions ----------------------------*/

void evaluateMotors() {
  static MotorMode_t lastMotorMode = MOTOR_STOPPED;
  bool repeatedMotorState = (lastMotorMode == currentMotorMovement);
  lastMotorMode = currentMotorMovement;
  switch(currentMotorMovement) {
    case MOTOR_STOPPED:
      motorsNSStop();
      motorsEWStop();
      break;
    case MOTOR_HUG_WEST_GOSOUTH:
      if(!repeatedMotorState) motorsNSSouth();
      if(!PIN_WEST_BUMPER) {
        motorsEWWest();
      } else {
        motorsEWStop();
      }
      break;
    case MOTOR_HUG_WEST_GONORTH:
      if(!repeatedMotorState) motorsNSNorth();
      if(!PIN_WEST_BUMPER) {
        motorsEWWest();
      } else {
        motorsEWStop();
      }
      break;
    case MOTOR_HUG_SOUTH_GOWEST:
      if(!repeatedMotorState) motorsEWWest();
      if(!PIN_SOUTH_BUMPER) {
        motorsNSSouth();
      } else {
        motorsNSStop();
      }
      break;
    }
}

void motorsNSNorth() {
  digitalWrite(PIN_NORTH_MOTOR_1, HIGH);
  digitalWrite(PIN_NORTH_MOTOR_2, LOW);
  digitalWrite(PIN_SOUTH_MOTOR_1, HIGH);
  digitalWrite(PIN_SOUTH_MOTOR_2, LOW);
}

void motorsNSStop() {
  digitalWrite(PIN_NORTH_MOTOR_1, LOW);
  digitalWrite(PIN_NORTH_MOTOR_2, LOW);
  digitalWrite(PIN_SOUTH_MOTOR_1, LOW);
  digitalWrite(PIN_SOUTH_MOTOR_2, LOW);
}

void motorsNSSouth() {
  digitalWrite(PIN_NORTH_MOTOR_1, LOW);
  digitalWrite(PIN_NORTH_MOTOR_2, HIGH);
  digitalWrite(PIN_SOUTH_MOTOR_1, LOW);
  digitalWrite(PIN_SOUTH_MOTOR_2, HIGH);
}

void motorsEWWest() {
  digitalWrite(PIN_EAST_MOTOR_1, HIGH);
  digitalWrite(PIN_EAST_MOTOR_2, LOW);
  digitalWrite(PIN_WEST_MOTOR_1, HIGH);
  digitalWrite(PIN_WEST_MOTOR_2, LOW);
}

void motorsEWStop() {
  digitalWrite(PIN_EAST_MOTOR_1, LOW);
  digitalWrite(PIN_EAST_MOTOR_2, LOW);
  digitalWrite(PIN_WEST_MOTOR_1, LOW);
  digitalWrite(PIN_WEST_MOTOR_2, LOW);
}

void motorsEWEast() {
  digitalWrite(PIN_EAST_MOTOR_1, LOW);
  digitalWrite(PIN_EAST_MOTOR_2, HIGH);
  digitalWrite(PIN_WEST_MOTOR_1, LOW);
  digitalWrite(PIN_WEST_MOTOR_2, HIGH);
}

/*-------------------------------- IO Functions ------------------------------*/

// Polls all 4 bumpers
void bumperPoll() {
  // Debouncing
  static uint16_t northState = 0;
  northState = ((northState << 1) | digitalRead(PIN_NORTH_BUMPER));
  static uint16_t southState = 0;
  southState = ((southState << 1) | digitalRead(PIN_SOUTH_BUMPER));
  static uint16_t eastState = 0;
  eastState = ((eastState << 1) | digitalRead(PIN_EAST_BUMPER));
  static uint16_t westState = 0;
  westState = ((northState << 1) | digitalRead(PIN_WEST_BUMPER));
  // Sets values
  northBumper = (northState == 0xFFFF);
  southBumper = (southState == 0xFFFF);
  eastBumper  = (eastState  == 0xFFFF);
  westBumper  = (westState  == 0xFFFF);
}

// Sensor subsystem says fire
void sensorISR() {
  if(fireEnabled) {
    cannonFire();
  }
}

// Start cannon motors spinning
void cannonSpool() {
  digitalWrite(PIN_CANNON_SPOOL, HIGH);
}

void cannonCooldown() {
  digitalWrite(PIN_CANNON_SPOOL, LOW);
}

// Fires all shots, make sure to spool up first
void cannonFire() {
  digitalWrite(PIN_CANNON_LAUNCH, HIGH); // Slave responds to rising edge
  delay(10);
  digitalWrite(PIN_CANNON_LAUNCH, LOW);
}
