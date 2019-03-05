#include <Arduino.h>
#include <Metro.h>
#include <IntervalTimer.h>

void checkGlobalEvents(void);
void handleMoveForward(void);
void handleMoveBackward(void);
void handleMoveWest(void);
void handleMoveEast(void);
void handleIdle(void);

int east_pin = 8; 
int west_pin = 7; 
int south_pin = 6; 
int north_pin = 5; 


int PIN_NORTH_MOTOR_1 = 4;
int PIN_NORTH_MOTOR_2 = 3;

int PIN_SOUTH_MOTOR_1 = 10;
int PIN_SOUTH_MOTOR_2 = 9;

int PIN_EAST_MOTOR_1 = 17;
int PIN_EAST_MOTOR_2 = 16;

int PIN_WEST_MOTOR_1 = 22;
int PIN_WEST_MOTOR_2 = 23;


bool motor_1_on = 0;
bool motor_1_north = 1;

bool motor_2_on = 0;
bool motor_2_west = 1;

typedef enum {
  STATE_MOVE_NORTH, STATE_MOVE_SOUTH, STATE_MOVE_WEST, STATE_MOVE_EAST, STATE_IDLE
} States_t;

States_t state;

bool LED_on = true;

bool south;
bool north;
bool west;
bool east;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_NORTH_MOTOR_1, OUTPUT);
  pinMode(PIN_NORTH_MOTOR_2, OUTPUT);
  digitalWrite(PIN_NORTH_MOTOR_1, motor_1_north);
  digitalWrite(PIN_NORTH_MOTOR_2, !motor_1_north);

  pinMode(PIN_SOUTH_MOTOR_1, OUTPUT);
  pinMode(PIN_SOUTH_MOTOR_2, OUTPUT);
  digitalWrite(PIN_SOUTH_MOTOR_1, motor_1_north);
  digitalWrite(PIN_SOUTH_MOTOR_2, !motor_1_north);


  pinMode(13, OUTPUT);
  digitalWrite(13, LED_on);

  pinMode(PIN_EAST_MOTOR_1, OUTPUT);
  pinMode(PIN_EAST_MOTOR_2, OUTPUT);
  digitalWrite(PIN_EAST_MOTOR_1, motor_2_west);
  digitalWrite(PIN_EAST_MOTOR_2, !motor_2_west);

  pinMode(PIN_WEST_MOTOR_1, OUTPUT);
  pinMode(PIN_WEST_MOTOR_2, OUTPUT);
  digitalWrite(PIN_WEST_MOTOR_1, motor_2_west);
  digitalWrite(PIN_WEST_MOTOR_2, !motor_2_west);

  pinMode(south_pin, INPUT_PULLUP);
  pinMode(north_pin, INPUT_PULLUP);
  pinMode(east_pin, INPUT_PULLUP);
  pinMode(west_pin, INPUT_PULLUP);

  // attachInterrupt(digitalPinToInterrupt(south_pin), count_falling_edges_south, RISING);
  // attachInterrupt(digitalPinToInterrupt(north_pin), count_falling_edges_north, RISING);
  // attachInterrupt(digitalPinToInterrupt(east_pin), count_falling_edges_east, RISING);
  // attachInterrupt(digitalPinToInterrupt(west_pin), count_falling_edges_west, RISING);

  state = STATE_IDLE;

 }


void loop() {
  static int count_print = 1;
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

  checkGlobalEvents();

  switch (state) {
    case STATE_MOVE_NORTH:
      handleMoveForward();
      break;
    case STATE_MOVE_SOUTH:
      handleMoveBackward();
      break;
    case STATE_MOVE_WEST:
      handleMoveWest();
      break;
    case STATE_MOVE_EAST:
      handleMoveEast();
      break;
    case STATE_IDLE:
      handleIdle();
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

    
}

 void handleMoveForward(void){
    motor_1_on = 1;
    //motor_2_on = 0;
    motor_1_north = 1;
    if(west == 0) {
      motor_2_on = 1;
      motor_2_west = 1;
      //Serial.println("hello");
    } else {
      motor_2_on = 0;
      motor_2_west = 1;
    }
    

 }
 void handleMoveBackward(void){
    motor_1_on = 1;
    //motor_2_on = 0;
    motor_1_north = 0;
  if(east == 0) {
      motor_2_on = 1;
      motor_2_west = 0;
    } else {
      motor_2_on = 0;
      motor_2_west = 0;
    }


 }
 void handleMoveWest(void){
    //motor_1_on = 0;
    motor_2_on = 1;
    motor_2_west = 1;
    if(south == 0) {
      motor_1_on = 1;
      motor_1_north = 0;
    } else {
      motor_1_on = 0;
      motor_1_north = 0;
    }

 }
 void handleMoveEast(void) {
       //motor_1_on = 0;
       motor_2_on = 1;
       motor_2_west = 0;
    if(north == 0) {
      motor_1_on = 1;
      motor_1_north = 1;
    } else {
      motor_1_on = 0;
      motor_1_north = 1;
    }

 }
 void handleIdle(void) {
      motor_1_on = 0;
      motor_2_on = 0;
      
 }

 void checkGlobalEvents(void) {


   if( (south && !west)|| (south && east) )  {
     state  = STATE_MOVE_WEST;
   } else if ( (west && !north )|| (west && south) ) {
      state  = STATE_MOVE_NORTH;
   } else if ( (north && !east) || (north && west) )  {
      state  = STATE_MOVE_EAST;
   } else if ((east && !south)|| (north && east) ) {
      state  = STATE_MOVE_SOUTH; 
   } 
   

 }


