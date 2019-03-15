/*
 * File: launcher
 * --------------
 * Scott Blankenberg, Perry Alagappan, Stephany, Erick Blankenberg
 * ME210
 * 2/5/2019
 * Final
 *
 * Description:
 * 	Implements detector
 */

/*------------------------------ Library Includes ----------------------------*/

#include <Arduino.h>
#include <IntervalTimer.h>

/*---------------------------- Function Definitions --------------------------*/

void count_falling_edges(void);
void determine_frequency(void);

/*------------------------------ Pin Definitions -----------------------------*/

#define PIN_SIGNAL_IN              4
#define PIN_SIGNAL_DETECTED_LED    0
#define PIN_SIGNAL_DETECTED_OUTPUT 18

/*----------------------------- Global Variables -----------------------------*/

int count = 0;

unsigned long rising_edge_start = 0;
unsigned long time_high = 0;
unsigned long avg_duty_cycle = 0;
bool freq_detected = false;
unsigned long reported_duty_cycle = 0;

IntervalTimer frequencyTimer;

/*----------------------------------- Main -----------------------------------*/

void setup() {
  Serial.begin(9600);

  pinMode(PIN_SIGNAL_DETECTED_OUTPUT, OUTPUT);
  pinMode(PIN_SIGNAL_DETECTED_LED, OUTPUT);
  pinMode(PIN_SIGNAL_IN, INPUT);

  frequencyTimer.begin(determine_frequency, 100000);
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN), count_falling_edges, CHANGE);
}

void loop() {
  digitalWrite(PIN_SIGNAL_DETECTED_LED, freq_detected);
  digitalWrite(PIN_SIGNAL_DETECTED_OUTPUT, freq_detected);
}

// Counts edges by incrementing on each call
void count_falling_edges(void) {
  bool value = digitalRead(PIN_SIGNAL_IN);
  if(value == 0) {
    //falling edge
    count = count+1;
    unsigned long curr_time = micros();
    time_high = curr_time - rising_edge_start;
    avg_duty_cycle = avg_duty_cycle + time_high;
  }
  else {
    //rising edge
    rising_edge_start = micros();
  }
}

// Calls Interrupt to Determine Number of Edges
void determine_frequency(void) {
  if (count >= 80 && count <= 200) {
    freq_detected = true;
    reported_duty_cycle = (100*avg_duty_cycle)/100000;
  } else {
    freq_detected = false;
  }
  avg_duty_cycle = 0;
  count = 0;
}
