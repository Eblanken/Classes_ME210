/*
 * File: launcher
 * --------------
 * Scott Blankenberg, Perry Alagappan, Stephany, Erick Blankenberg
 * ME210
 * 2/5/2019
 * Final Project
 *
 * Description:
 * 	Implements Launcher Code
 */

/*------------------------------ Library Includes ----------------------------*/

#include <Arduino.h>
#include <Servo.h>

/*------------------------------ Pin Definitions -----------------------------*/

#define PIN_LAUNCHERTOP     22
#define PIN_LAUNCHERBOTTOM  23
#define PIN_INPUT_STARTMOTOR      16
#define PIN_INPUT_ACTUATESERVO    17
#define PIN_SERVO           3

/*------------------------------ Global Variables ----------------------------*/

Servo myservo;
bool value = false;

/*----------------------------------- Main -----------------------------------*/

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LAUNCHERTOP, OUTPUT);
  pinMode(PIN_LAUNCHERBOTTOM, OUTPUT);
  pinMode(PIN_INPUT_STARTMOTOR, INPUT_PULLDOWN);
  pinMode(PIN_INPUT_ACTUATESERVO, INPUT_PULLDOWN);
  myservo.attach(PIN_SERVO);
  myservo.write(0);
}

void loop() {
  // Spins up launcher
  if(digitalRead(PIN_INPUT_STARTMOTOR)) {
    analogWrite(PIN_LAUNCHERTOP, 200);
    analogWrite(PIN_LAUNCHERBOTTOM, 200);
  } else {
    analogWrite(PIN_LAUNCHERTOP, 0);
    analogWrite(PIN_LAUNCHERBOTTOM, 0);
  }
  // Releases ammunition
  if(digitalRead(PIN_INPUT_ACTUATESERVO)) {
       myservo.write(180);
       delay(3000);
       myservo.write(0);
  }
}
