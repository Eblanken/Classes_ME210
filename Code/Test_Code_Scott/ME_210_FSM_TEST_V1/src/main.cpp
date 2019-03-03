#include <Arduino.h>
#include <Metro.h>
#include <IntervalTimer.h>


void checkGlobalEvents(void);
void handleMoveNorth(void);
void handleMoveSouth(void);
void handleMoveWest(void);
void handleMoveEast(void);
void hugNorthWall(void);
void hugSouthWall(void);
void hugEastWall(void);
void hugWestWall(void);
void handleIdle(void);
uint8_t check_reload_expired(void);
void resp_reload_Timer_expired(void);
uint8_t check_fire_expired(void);
void resp_fire_Timer_expired(void);

//void count_falling_edges_1(void);
//void determine_frequency_1(void);
//void count_falling_edges_2(void);
//void determine_frequency_2(void);

int east_pin = 2;
int west_pin = 3;
int south_pin = 4;
int north_pin = 5;

int PIN_MOTOR_11 = 23;
int PIN_MOTOR_12 = 22;

int PIN_MOTOR_21 = 18;
int PIN_MOTOR_22 = 17;

//int count_1 = 0;
//int count_2 = 0;

//bool freq_1_detected = false;
//bool freq_2_detected = false;

//IntervalTimer frequencyTimer_1;
//IntervalTimer frequencyTimer_2;

//int PIN_SIGNAL_IN = 21;
int LED_PIN = 16;
//int PIN_SIGNAL_IN_2 = 20;
int LED_PIN_2 = 15;

bool motor_1_on = 0;
bool motor_1_north = 1;

bool motor_2_on = 0;
bool motor_2_west = 1;

int RELOAD_TIME = 5E3;

static Metro reloadTimer = Metro(RELOAD_TIME);

int FIRE_TIME = 5E3;

static Metro fireTimer = Metro(FIRE_TIME);


typedef enum {
  INIT_WEST, INIT_NORTH, RELOAD, MOVE_EAST, SCAN_SOUTH, FIRE, SCAN_NORTH, RELOAD_NORTH, RELOAD_WEST
} States_t;

States_t state;

bool pin_1_in_LED;
bool pin_2_in_LED;
bool south;
bool north;
bool west;
bool east;

bool LED_on = true;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, INPUT);
  pinMode(LED_PIN_2, INPUT);

  pinMode(PIN_MOTOR_11, OUTPUT);
  pinMode(PIN_MOTOR_12, OUTPUT);
  digitalWrite(PIN_MOTOR_11, motor_1_north);
  digitalWrite(PIN_MOTOR_12, !motor_1_north);
  pinMode(13, OUTPUT);
  digitalWrite(13, LED_on);

  pinMode(PIN_MOTOR_21, OUTPUT);
  pinMode(PIN_MOTOR_22, OUTPUT);
  digitalWrite(PIN_MOTOR_21, motor_2_west);
  digitalWrite(PIN_MOTOR_22, !motor_2_west);

  pinMode(south_pin, INPUT_PULLUP);
  pinMode(north_pin, INPUT_PULLUP);
  pinMode(east_pin, INPUT_PULLUP);
  pinMode(west_pin, INPUT_PULLUP);

  state = INIT_WEST;

 }


void loop() {
  static int count_print = 1;
  pin_1_in_LED = digitalRead(LED_PIN);
  pin_2_in_LED = digitalRead(LED_PIN_2);
  south =  digitalRead(south_pin);
  north =  digitalRead(north_pin);
  west =  digitalRead(west_pin);
  east =  digitalRead(east_pin);
  if(count_print == 1000) {
    Serial.print("State = ");
    Serial.println(state);
    Serial.print("east_pin = ");
    Serial.println(east);
    Serial.print("west_pin = ");
    Serial.println(west);
    Serial.print("north_pin = ");
    Serial.println(north);
    Serial.print("south_pin = ");
    Serial.println(south);
    count_print = 1;
    Serial.print("motor_1_on = ");
    Serial.print(motor_1_on);
    Serial.print(" motor_1_north = ");
    Serial.print(motor_1_north);
    Serial.print(" motor_2_on = ");
    Serial.print(motor_2_on);
    Serial.print(" motor_2_west = ");
    Serial.println(motor_2_west);
    
  }
  count_print = count_print+1;


  switch (state) {
    case INIT_WEST:
      handleMoveWest();
      hugSouthWall();
      if((west && !north )|| (west && south) ) {
          state = INIT_NORTH;
      }
      break;
    case INIT_NORTH:
      handleMoveNorth();
      hugWestWall();
      if ((north && !east) || (north && west) ) {
        state = RELOAD;
        reloadTimer.reset();
      }
      break;
    case RELOAD:
      handleIdle();
      if(check_reload_expired()) {
        state = MOVE_EAST;
      }
      break;
    case MOVE_EAST:
      handleMoveEast();
      hugNorthWall();
      if ( (east && !south) || (north && east)) {
        state = SCAN_SOUTH;
      }
      break;
    case SCAN_SOUTH:
      handleMoveSouth();
      hugEastWall();
      if(pin_1_in_LED || pin_2_in_LED) {
        state = FIRE;
        fireTimer.reset();
      }
      if( (south && !west) || (south && east)  ) {
        state = SCAN_NORTH;
      }
      break;
    case FIRE:
      handleIdle();
      if(check_fire_expired()) {
        state = RELOAD_NORTH;
      }
      break;

    case SCAN_NORTH:
      handleMoveNorth();
      hugEastWall();
      if(pin_1_in_LED || pin_2_in_LED) {
        state = FIRE;
        fireTimer.reset();
      }
      if((north && !west) || (north && east)) {
        state = SCAN_SOUTH;
      }
      break;

    case RELOAD_NORTH:
      handleMoveNorth();
      hugEastWall();
      if(  (north && !west) || (north && east)  ) {
        state = RELOAD_WEST;
      }
      break;

    case RELOAD_WEST:
      handleMoveWest();
      hugNorthWall();
      if( (west && !south) || (west && north)) {
        state = RELOAD;
        reloadTimer.reset();
      }
      break;
    default:    // Should never get into an unhandled state
      Serial.println("What is this I do not even...");
  }


  if(motor_1_on) {
      digitalWrite(PIN_MOTOR_11, motor_1_north);
      digitalWrite(PIN_MOTOR_12, !motor_1_north);
  }
  else {
      digitalWrite(PIN_MOTOR_11, 0);
      digitalWrite(PIN_MOTOR_12, 0);
  }

  if(motor_2_on ) {
      digitalWrite(PIN_MOTOR_21, motor_2_west);
      digitalWrite(PIN_MOTOR_22, !motor_2_west);
  }
  else {
      digitalWrite(PIN_MOTOR_21, 0);
      digitalWrite(PIN_MOTOR_22, 0);
  }
}

 uint8_t check_reload_expired(void) {
  return (uint8_t) reloadTimer.check();
}

 uint8_t check_fire_expired(void) {
  return (uint8_t) fireTimer.check();
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
    if(north == 0) {
      motor_1_on = 1;
      motor_1_north = 1;
    } else {
      motor_1_on = 0;
      motor_1_north = 1;
    }

 }
void hugSouthWall(void) {
  if(south == 0) {
    motor_1_on = 1;
    motor_1_north = 0;
  } else {
    motor_1_on = 0;
    motor_1_north = 0;
  }

}
void hugEastWall(void) {
  if(east == 0) {
    motor_2_on = 1;
    motor_2_west = 0;
  } else {
    motor_2_on = 0;
    motor_2_west = 0;
    }

}
void hugWestWall(void) {
  if(west == 0) {
    motor_2_on = 1;
    motor_2_west = 1;     
  } else {
    motor_2_on = 0;
    motor_2_west = 1;
  }
}