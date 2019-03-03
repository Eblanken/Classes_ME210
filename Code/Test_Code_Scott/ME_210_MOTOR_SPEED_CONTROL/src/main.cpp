#include <Arduino.h>
#include <Metro.h>
#include <IntervalTimer.h>
#include <AccelStepper.h>


uint8_t TestForKey(void);

void RespToKey(void);

void CountRisingEdges(void);

void printFrequency(void);



volatile signed int edge_count = 0;

int DESIRED_RPM = 700;

int rpm = 0;

int duty_cycle;
// digital output pin
int PIN_OUT = 20;

// pin connected to wiper of POTENTIOMETER
int PIN_POTENTIOMETER = 17;

int DIR = 18;

int dir_value = 0;

int PIN_A = 21;

int PIN_B = 22;

int EDGE_TIMER_INTERVAL = 100000;

IntervalTimer PRINT_EDGE_TIMER;


void setup() {
  Serial.begin(9600);

  pinMode(PIN_OUT , OUTPUT);

  pinMode(DIR, OUTPUT);

  //pinMode(PIN_POTENTIOMETER , INPUT);

  pinMode(PIN_A , INPUT);

  pinMode(PIN_B , INPUT);

  PRINT_EDGE_TIMER.begin(printFrequency, EDGE_TIMER_INTERVAL);

  attachInterrupt(digitalPinToInterrupt(PIN_A), CountRisingEdges, RISING);

}

/*
Based on the voltage at the wiper, set the frequency of the digital output square wave. 
*/
void loop() {

  //int voltage = analogRead(PIN_POTENTIOMETER);
  //duty_cycle = map(voltage, 0, 1023, 0, 255);
  analogWrite(PIN_OUT, duty_cycle);
  if (TestForKey()) RespToKey();

}

void CountRisingEdges(void) {

  detachInterrupt(PIN_A);

  int value_B = digitalRead(PIN_B);

  if(value_B == 1) {
    edge_count = edge_count+1;

  }
  else if(value_B == 0){
      edge_count = edge_count-1;

      
  }
  
  attachInterrupt(digitalPinToInterrupt(PIN_A), CountRisingEdges, RISING);

}

void printFrequency(void) {
    detachInterrupt(PIN_A);
    Serial.print("Edge Count = ");
    Serial.println(edge_count);
    

    rpm = int(abs(edge_count)*600)/48;
    Serial.print("RPM = ");
    Serial.println(rpm);


    if(rpm < DESIRED_RPM && duty_cycle < 255) {
      duty_cycle = duty_cycle+1;
    } else if(rpm > DESIRED_RPM && duty_cycle > 0  ){
      duty_cycle = duty_cycle-1;
    }

    int PWM = map(duty_cycle, 0, 255, 0, 100);

    Serial.print("PWM = ");
    Serial.println(PWM);

    Serial.print("dir = ");
    Serial.println(dir_value);
    edge_count = 0;
    
    attachInterrupt(digitalPinToInterrupt(PIN_A), CountRisingEdges, RISING);
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
           dir_value = !dir_value;
           digitalWriteFast(DIR, dir_value);


      }     
}

