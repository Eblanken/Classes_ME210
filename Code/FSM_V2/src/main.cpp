#include <Arduino.h>
#include <Metro.h>


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
uint8_t check_reload_expired(void);
void resp_reload_Timer_expired(void);
uint8_t check_fire_expired(void);
void resp_fire_Timer_expired(void);


uint8_t check_shoot_ok_expired(void);

uint8_t check_south_expired(void);
void resp_south_Timer_expired(void);

bool south;
bool north;
bool west;
bool east;

bool photo_in_1;
bool photo_in_2;


int east_pin = 8; 
int west_pin = 7; 
int south_pin = 6; 
int north_pin = 5; 

int OUTPUT_LAUNCHER_MOTOR = 19;
int OUTPUT_SERVO = 18;


int PIN_NORTH_MOTOR_1 = 4;
int PIN_NORTH_MOTOR_2 = 3;

int PIN_SOUTH_MOTOR_1 = 10;
int PIN_SOUTH_MOTOR_2 = 9;

int PIN_EAST_MOTOR_1 = 17;
int PIN_EAST_MOTOR_2 = 16;

int PIN_WEST_MOTOR_1 = 22;
int PIN_WEST_MOTOR_2 = 23;

//int SERVO_PIN = 20;

int PHOTO_SIGNAL_IN = 12;

bool motor_1_on = 0;
bool motor_1_north = 1;

bool motor_2_on = 0;
bool motor_2_west = 1;


bool ok_to_shoot = false;

bool LED_on = true;

int RELOAD_TIME = 5E3;

static Metro reloadTimer = Metro(RELOAD_TIME);

int FIRE_TIME = 5E3;

static Metro fireTimer = Metro(FIRE_TIME);

int GAME_TIME = 130e3;

static Metro gameTimer = Metro(GAME_TIME);

int SHOOT_OK_TIME = 1e3;

static Metro shootOkTimer = Metro(SHOOT_OK_TIME);

bool fired_shot = true;

bool fire =false;


typedef enum {
  IDLE_STATE, INIT_WEST, NORTH, ARMORY_STATE, SOUTH, FIRE_STATE
} States_t;

States_t state;



void setup() {
  Serial.begin(9600);

  //launcher pins
  pinMode(OUTPUT_LAUNCHER_MOTOR, OUTPUT);
  digitalWrite(OUTPUT_LAUNCHER_MOTOR, LOW);

  pinMode(OUTPUT_SERVO, OUTPUT);
  digitalWrite(OUTPUT_SERVO, LOW);


  //Phototransistor pins
  pinMode(PHOTO_SIGNAL_IN, INPUT_PULLDOWN);


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


  //switch pins for drive train
  pinMode(south_pin, INPUT_PULLUP);
  pinMode(north_pin, INPUT_PULLUP);
  pinMode(east_pin, INPUT_PULLUP);
  pinMode(west_pin, INPUT_PULLUP);


  //debug LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LED_on);


  //Initial states for drive train and tower
  state = INIT_WEST;

}

void loop() {

  
  if(ok_to_shoot) {
      digitalWrite(OUTPUT_LAUNCHER_MOTOR, HIGH);
  }
  else {
    digitalWrite(OUTPUT_LAUNCHER_MOTOR, LOW);
  }

  static int count_print = 1;
  south =  digitalRead(south_pin);
  north =  digitalRead(north_pin);
  west =  digitalRead(west_pin);
  east =  digitalRead(east_pin);
  photo_in_1 = digitalRead(PHOTO_SIGNAL_IN);

  //fire = (photo_in_1);
  int val_test = ok_to_shoot;

  if(count_print == 1000) {
    Serial.print("State = ");
    Serial.println(state);
    Serial.print("Fire = ");
    Serial.println(fire);
    Serial.print("fired = ");
    Serial.println(fired_shot);
    Serial.print("ok_to_shoot = ");
    Serial.println(ok_to_shoot);
    count_print = 1;

    // Serial.print("motor_out = ");
    // Serial.println(val_test);
    // Serial.print("east_pin = ");
    // Serial.println(east);
    // Serial.print("west_pin = ");
    // Serial.println(west);
    // Serial.print("north_pin = ");
    // Serial.println(north);
    // Serial.print("south_pin = ");
    // Serial.println(south);
    // Serial.print("motor_1_on = ");
    // Serial.print(motor_1_on);
    // Serial.print(" motor_1_north = ");
    // Serial.print(motor_1_north);
    // Serial.print(" motor_2_on = ");
    // Serial.print(motor_2_on);
    // Serial.print(" motor_2_west = ");
    // Serial.println(motor_2_west);
    
  }
  count_print = count_print+1;

  switch (state) {
    case INIT_WEST:
      handleMoveWest();
      hugSouthWall();
      if((west && !north )|| (west && south) ) {
          state = NORTH;
      }
      break;
    case NORTH:
      handleMoveNorth();
      hugWestWall();
      if ((north && !east) || (north && west) ) {
        if(fired_shot) {
          state = ARMORY_STATE;
          reloadTimer.reset();
        } else {
          state = SOUTH;
          shootOkTimer.reset();
        }
      }
      else if(!fired_shot && fire && ok_to_shoot) {
        state = FIRE_STATE;
        fireTimer.reset();
      } 
      break;
    case ARMORY_STATE:
      handleIdle();
      if(check_reload_expired()) {
        state = SOUTH;
        shootOkTimer.reset();
        fired_shot = false;
      }
      break;
    case SOUTH:
      handleMoveSouth();
      hugWestWall();
      if((south && !east) || (south && west)) {
        state = NORTH;
      }
      else if(fire && ok_to_shoot) {
        state = FIRE_STATE;
        fireTimer.reset();
      } 
      ok_to_shoot = check_shoot_ok_expired();
      break;
    case FIRE_STATE:
      handleIdle();
      digitalWrite(OUTPUT_SERVO, HIGH);
      if(check_fire_expired()) {
        ok_to_shoot = false;
        fired_shot = true;
        state = NORTH;
        digitalWrite(OUTPUT_SERVO, LOW);
      }
      break;

    default:    // Should never get into an unhandled state
      Serial.println("What is this I do not even...");
  }


  if(motor_1_on) {
      digitalWrite(PIN_NORTH_MOTOR_1, motor_1_north);
      digitalWrite(PIN_NORTH_MOTOR_2, !motor_1_north);
      digitalWrite(PIN_SOUTH_MOTOR_1, motor_1_north);
      digitalWrite(PIN_SOUTH_MOTOR_2, !motor_1_north);
  }
  else {
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
  }
  else {
      digitalWrite(PIN_EAST_MOTOR_1, 0);
      digitalWrite(PIN_EAST_MOTOR_2, 0);
      digitalWrite(PIN_WEST_MOTOR_1, 0);
      digitalWrite(PIN_WEST_MOTOR_2, 0);
  }

  attachInterrupt(digitalPinToInterrupt(PHOTO_SIGNAL_IN), edge_response, CHANGE);

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

 uint8_t check_reload_expired(void) {
  return (uint8_t) reloadTimer.check();
}

 uint8_t check_fire_expired(void) {
  return (uint8_t) fireTimer.check();
 }

  uint8_t check_shoot_ok_expired(void) {
  return (uint8_t) shootOkTimer.check();
 }

 void edge_response(void) {
   detachInterrupt(PHOTO_SIGNAL_IN);
   if(digitalRead(PHOTO_SIGNAL_IN)) {
      //rising edge
       fire = true;
   }
   else {
     //falling edge
     fire = false;
   }
    
  attachInterrupt(digitalPinToInterrupt(PHOTO_SIGNAL_IN), edge_response, CHANGE);
   
 }
