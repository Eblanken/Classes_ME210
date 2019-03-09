#include <Arduino.h>
#include <Metro.h>
#include <IntervalTimer.h>
#include <AccelStepper.h>


uint8_t TestForKey(void);
void RespToKey(void);

void CountRisingEdges_TOP(void);
void CountRisingEdges_BOTTOM(void);

void printFrequency(void);
uint8_t Test_ENCODER_TOP_TIMER_EXPIRED(void);
uint8_t Test_ENCODER_BOTTOM_TIMER_EXPIRED(void);

void RESP_ENCODER_TOP_TIMER_EXPIRED(void);

void RESP_ENCODER_BOTTOM_TIMER_EXPIRED(void);


#define PIN_LAUNCHERTOP     22
#define PIN_LAUNCHERBOTTOM  23
#define ENCODER_INPUT_TOP  20
#define ENCODER_INPUT_BOTTOM 19


volatile signed int edge_count_TOP = 0;
volatile signed int edge_count_BOTTOM = 0;

int DESIRED_RPS_TOP = 30;
int DESIRED_RPS_BOTTOM = 30;

int rps_TOP = 0;
int rps_BOTTOM = 0;

int duty_cycle_TOP = 75;
int duty_cycle_BOTTOM = 75;

int EDGE_TIMER_INTERVAL = 1E3;

static Metro ENCODER_TOP_TIMER = Metro(100);

static Metro ENCODER_BOTTOM_TIMER = Metro(100);

IntervalTimer freqTimer;


void setup() {
  Serial.begin(9600);
  pinMode(PIN_LAUNCHERTOP, OUTPUT);
  pinMode(PIN_LAUNCHERBOTTOM, OUTPUT);
  pinMode(ENCODER_INPUT_TOP , INPUT);
  pinMode(ENCODER_INPUT_BOTTOM , INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  //pinMode(MASTER_INPUT_ONE , INPUT);
  //pinMode(MASTER_INPUT_TWO , INPUT);


  //freqTimer.begin(printFrequency, EDGE_TIMER_INTERVAL);


  //attachInterrupt(digitalPinToInterrupt(ENCODER_INPUT_TOP), CountRisingEdges_TOP, RISING);
  //attachInterrupt(digitalPinToInterrupt(ENCODER_INPUT_BOTTOM), CountRisingEdges_BOTTOM, RISING);

}

/*
Based on the voltage at the wiper, set the frequency of the digital output square wave. 
*/
void loop() {


  //if (TestForKey()) RespToKey();


  // if(Test_ENCODER_TOP_TIMER_EXPIRED()) { 
  //   RESP_ENCODER_TOP_TIMER_EXPIRED();
  // }
  //  if(Test_ENCODER_BOTTOM_TIMER_EXPIRED()) { 
  //   RESP_ENCODER_BOTTOM_TIMER_EXPIRED();
  // }

}


void printFrequency(void) {
    detachInterrupt(ENCODER_INPUT_TOP);
    detachInterrupt(ENCODER_INPUT_BOTTOM);
    

    if(rps_TOP < DESIRED_RPS_TOP && duty_cycle_TOP < 255) {
      duty_cycle_TOP = duty_cycle_TOP+1;
    } else if(rps_TOP > DESIRED_RPS_TOP && duty_cycle_TOP > 0  ){
      duty_cycle_TOP = duty_cycle_TOP-1;
    }

    if(rps_BOTTOM < DESIRED_RPS_BOTTOM && duty_cycle_BOTTOM < 255) {
      duty_cycle_BOTTOM = duty_cycle_BOTTOM+1;
    } else if(rps_BOTTOM > DESIRED_RPS_BOTTOM && duty_cycle_BOTTOM > 0  ){
      duty_cycle_BOTTOM = duty_cycle_BOTTOM-1;
    }

    int PWM_TOP = map(duty_cycle_TOP, 0, 255, 0, 100);

     int PWM_BOTTOM = map(duty_cycle_BOTTOM, 0, 255, 0, 100);


    Serial.print("RPS_TOP = ");
    Serial.println(rps_TOP);      
    Serial.print("RPS_BOTTOM = ");
    Serial.println(rps_BOTTOM);
      

    Serial.print("PWM_TOP = ");
    Serial.println(PWM_TOP);

    // Serial.print("duty_cycle_TOP = ");
    // Serial.println(duty_cycle_TOP);

    Serial.print("PWM_BOTTOM = ");
    Serial.println(PWM_BOTTOM);

    // Serial.print("duty_cycle_BOTTOM = ");
    // Serial.println(duty_cycle_BOTTOM);
    
    
    attachInterrupt(digitalPinToInterrupt(ENCODER_INPUT_TOP), CountRisingEdges_TOP, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_INPUT_BOTTOM), CountRisingEdges_BOTTOM, RISING);
}


uint8_t TestForKey(void) {
  uint8_t KeyEventOccurred;
  KeyEventOccurred = Serial.available();
  return KeyEventOccurred;
}

void RespToKey(void) {
      uint8_t theKey;
      theKey = Serial.read();
      if(theKey == 'a') {
        Serial.println("key pressed");

  
      }     
}


uint8_t Test_ENCODER_TOP_TIMER_EXPIRED(void) {
  return (uint8_t) ENCODER_TOP_TIMER.check();
}

uint8_t Test_ENCODER_BOTTOM_TIMER_EXPIRED(void) {
  return (uint8_t) ENCODER_BOTTOM_TIMER.check();
}

void RESP_ENCODER_TOP_TIMER_EXPIRED(void) {
  rps_TOP = edge_count_TOP*10;
  edge_count_TOP = 0;
  ENCODER_TOP_TIMER.reset();

}

void RESP_ENCODER_BOTTOM_TIMER_EXPIRED(void) {
  rps_BOTTOM = edge_count_BOTTOM*10;
  edge_count_BOTTOM = 0;
  ENCODER_BOTTOM_TIMER.reset();
}


void CountRisingEdges_TOP(void) {
  edge_count_TOP++;

}
void CountRisingEdges_BOTTOM(void) {
   edge_count_BOTTOM++;
}