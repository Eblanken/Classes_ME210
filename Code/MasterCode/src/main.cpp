/*
 * File: main
 * --------------
 * Scott Blankenberg, Perry Alagappan, Stephany, Erick Blankenberg
 * ME210
 * 2/5/2019
 * Final
 *
 * Description:
 * 	Implements Master FSM
 */

/*------------------------------ Library Includes ----------------------------*/

#include <Arduino.h>
#include <Metro.h>

/*---------------------------- Function Definitions --------------------------*/

// Motor functions
void handleMoveNorth(void);
void handleMoveSouth(void);
void handleMoveWest(void);
void handleMoveEast(void);
void hugNorthWall(void);
void hugSouthWall(void);
void hugEastWall(void);
void hugWestWall(void);
void handleIdle(void);
void edge_response(void);

// Timer functions
uint8_t check_reload_expired(void);
uint8_t check_south_expired(void);
uint8_t check_game_expired(void);

/*------------------------------ Pin Definitions -----------------------------*/

// Bumper pins, make sure to use input pullup. Is high when depressed.
#define PIN_EAST_BUMPER  8
#define PIN_WEST_BUMPER  7
#define PIN_SOUTH_BUMPER 6
#define PIN_NORTH_BUMPER 5

// IO, write launcher high to start spinning, servo high to release ammo
// rising interrupt on PHOTO_SIGNAL_IN.
#define OUTPUT_LAUNCHER_MOTOR 19
#define OUTPUT_SERVO          18
#define PHOTO_SIGNAL_IN 12

// Motor pins
#define PIN_NORTH_MOTOR_1 4
#define PIN_NORTH_MOTOR_2 3
#define PIN_SOUTH_MOTOR_1 10
#define PIN_SOUTH_MOTOR_2 9
#define PIN_EAST_MOTOR_1  17
#define PIN_EAST_MOTOR_2  16
#define PIN_WEST_MOTOR_1  22
#define PIN_WEST_MOTOR_2  23

/*----------------------------- Global Variables -----------------------------*/

// Bumper tracking
bool bumper_south_down;
bool bumper_north_down;
bool bumper_west_down;
bool bumper_east_down;

// Motor state
bool motor_1_on = 0;
bool motor_1_north = 1;
bool motor_2_on = 0;
bool motor_2_west = 1;

// Timers
#define RELOAD_TIME 5E3 // (ms) pause time for human reload
static Metro reloadTimer = Metro(RELOAD_TIME);
#define GAME_TIME 130e3 // (ms) total game time, bot shuts down after
static Metro gameTimer = Metro(GAME_TIME);
#define SOUTH_TIME 1500 // (ms) time for driving away from armoury to dragon spot
static Metro southTimer = Metro(SOUTH_TIME);

// Launcher controls
bool fired_shot = true;
bool safety_off =false;

// State data
typedef enum {
  IDLE_STATE, INIT_WEST, NORTH, ARMORY_STATE, SOUTH, END_GAME
} States_t;
States_t state;

/*----------------------------------- Main -----------------------------------*/

void setup() {
  Serial.begin(9600);

  // Launcher pins
  pinMode(OUTPUT_LAUNCHER_MOTOR, OUTPUT);
  digitalWrite(OUTPUT_LAUNCHER_MOTOR, LOW);
  pinMode(OUTPUT_SERVO, OUTPUT);
  digitalWrite(OUTPUT_SERVO, LOW);

  //Phototransistor pins
  pinMode(PHOTO_SIGNAL_IN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(PHOTO_SIGNAL_IN), edge_response, RISING);

  //Drive train pins
  pinMode(PIN_NORTH_MOTOR_1, OUTPUT);
  pinMode(PIN_NORTH_MOTOR_2, OUTPUT);
  digitalWrite(PIN_NORTH_MOTOR_1, motor_1_north);
  digitalWrite(PIN_NORTH_MOTOR_2, !motor_1_north);
  pinMode(PIN_SOUTH_MOTOR_1, OUTPUT);
  pinMode(PIN_SOUTH_MOTOR_2, OUTPUT);
  digitalWrite(PIN_SOUTH_MOTOR_1, motor_1_north);
  digitalWrite(PIN_SOUTH_MOTOR_2, !motor_1_north);
  pinMode(PIN_EAST_MOTOR_1, OUTPUT);
  pinMode(PIN_EAST_MOTOR_2, OUTPUT);
  digitalWrite(PIN_EAST_MOTOR_1, motor_2_west);
  digitalWrite(PIN_EAST_MOTOR_2, !motor_2_west);
  pinMode(PIN_WEST_MOTOR_1, OUTPUT);
  pinMode(PIN_WEST_MOTOR_2, OUTPUT);
  digitalWrite(PIN_WEST_MOTOR_1, motor_2_west);
  digitalWrite(PIN_WEST_MOTOR_2, !motor_2_west);

  // Bumper pins
  pinMode(PIN_SOUTH_BUMPER, INPUT_PULLUP);
  pinMode(PIN_NORTH_BUMPER, INPUT_PULLUP);
  pinMode(PIN_EAST_BUMPER, INPUT_PULLUP);
  pinMode(PIN_WEST_BUMPER, INPUT_PULLUP);

  // Initial states for drive train and tower
  state = INIT_WEST;
}

void loop() {
  if (check_game_expired()) {
    state = END_GAME;
  }

  static int count_print = 1;
  bumper_south_down =  digitalRead(PIN_SOUTH_BUMPER);
  bumper_north_down =  digitalRead(PIN_NORTH_BUMPER);
  bumper_west_down =  digitalRead(PIN_WEST_BUMPER);
  bumper_east_down =  digitalRead(PIN_EAST_BUMPER);

  if(count_print == 1000) {
    Serial.print("State = ");
    Serial.println(state);
    Serial.print("safety_off = ");
    Serial.println(safety_off);
    Serial.print("fired = ");
    Serial.println(fired_shot);
    count_print = 1;
  }

  count_print = count_print+1;

  switch (state) {
    case INIT_WEST:
      handleMoveWest();
      hugSouthWall();
      if((bumper_west_down && !bumper_north_down )|| (bumper_west_down && bumper_south_down) ) {
          state = NORTH;
      }
      break;
    case NORTH:
      handleMoveNorth();
      hugWestWall();
      if ((bumper_north_down && !bumper_east_down) || (bumper_north_down && bumper_west_down) ) {
          state = ARMORY_STATE;
          reloadTimer.reset();
      }
      break;

    case ARMORY_STATE:
      handleIdle();
      if(check_reload_expired()) {
        state = SOUTH; //PREVIOUS STATE SOUTH
        southTimer.reset();
        digitalWrite(OUTPUT_LAUNCHER_MOTOR, HIGH);
      }
      break;

    case SOUTH:
      handleMoveSouth();
      hugWestWall();
       if (check_south_expired()){
        state = IDLE_STATE;
        safety_off = true;
      }
    break;

    case IDLE_STATE:
      handleIdle();
      if(!safety_off) { // Gun fired
        delay(3000);
        digitalWrite(OUTPUT_LAUNCHER_MOTOR, LOW);
        state = NORTH;
      }
      break;

    case END_GAME:
      handleIdle();
      state = END_GAME;
      digitalWrite(OUTPUT_LAUNCHER_MOTOR, LOW);
      break;

    default:    // Should never get into an unhandled state
      Serial.println("What is this I do not even...");
  }

  if(motor_1_on) {
      digitalWrite(PIN_NORTH_MOTOR_1, motor_1_north);
      digitalWrite(PIN_NORTH_MOTOR_2, !motor_1_north);
      digitalWrite(PIN_SOUTH_MOTOR_1, motor_1_north);
      digitalWrite(PIN_SOUTH_MOTOR_2, !motor_1_north);
  } else {
      digitalWrite(PIN_NORTH_MOTOR_1, 0);
      digitalWrite(PIN_NORTH_MOTOR_2, 0);
      digitalWrite(PIN_SOUTH_MOTOR_1, 0);
      digitalWrite(PIN_SOUTH_MOTOR_2, 0);
  }

  if(motor_2_on ) {
      digitalWrite(PIN_EAST_MOTOR_1, motor_2_west);
      digitalWrite(PIN_EAST_MOTOR_2, !motor_2_west);
      digitalWrite(PIN_WEST_MOTOR_1, motor_2_west);
      digitalWrite(PIN_WEST_MOTOR_2, !motor_2_west);
  } else {
      digitalWrite(PIN_EAST_MOTOR_1, 0);
      digitalWrite(PIN_EAST_MOTOR_2, 0);
      digitalWrite(PIN_WEST_MOTOR_1, 0);
      digitalWrite(PIN_WEST_MOTOR_2, 0);
  }
}


 void handleMoveNorth(void){
  motor_1_on = 1;
  motor_1_north = 1;
 }

 void handleMoveSouth(void){
  motor_1_on = 1;
  motor_1_north = 0;
 }

 void handleMoveWest(void){
  motor_2_on = 1;
  motor_2_west = 1;
 }

 void handleMoveEast(void) {
  motor_2_on = 1;
  motor_2_west = 0;
 }

 void handleIdle(void) {
  motor_1_on = 0;
  motor_2_on = 0;
 }

 void hugNorthWall(void) {
  if(bumper_north_down == 0) {
    motor_1_on = 1;
    motor_1_north = 1;
  } else {
    motor_1_on = 0;
    motor_1_north = 1;
  }
}

void hugSouthWall(void) {
  if(bumper_south_down == 0) {
    motor_1_on = 1;
    motor_1_north = 0;
  } else {
    motor_1_on = 0;
    motor_1_north = 0;
  }

}

void hugEastWall(void) {
  if(bumper_east_down == 0) {
    motor_2_on = 1;
    motor_2_west = 0;
  } else {
    motor_2_on = 0;
    motor_2_west = 0;
    }

}

void hugWestWall(void) {
  if(bumper_west_down == 0) {
    motor_2_on = 1;
    motor_2_west = 1;
  } else {
    motor_2_on = 0;
    motor_2_west = 1;
  }
}

uint8_t check_reload_expired(void) {
  return (uint8_t) reloadTimer.check();
}

uint8_t check_south_expired(void) {
  return (uint8_t) southTimer.check();
}

uint8_t check_game_expired(void) {
  return (uint8_t) gameTimer.check();
}

void edge_response(void) {
  if(safety_off) {
    safety_off = false;
    digitalWrite(OUTPUT_SERVO, HIGH);
    delay(15);
    digitalWrite(OUTPUT_SERVO, LOW);
  }
}
