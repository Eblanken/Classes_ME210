/*
 * File: main.cpp
 * --------------
 * Erick Blankenberg
 * ME210
 * 1/20/2019
 * Lab 1
 *
 * Description:
 * 	Implements launcher
 */

/*
 * Serial interface:
 *
 */

#include <Arduino.h>
#include <i2c_t3.h>
#include <PWMServo.h>

/*--------------------------------- Constants --------------------------------*/

#define PIN_LED             13
#define PIN_SERVO           3
#define PIN_LAUNCHERTOP     23
#define PIN_LAUNCHERBOTTOM  24
#define PIN_SENSORTOP       21
#define PIN_SENSORBOTTOM    20

#define DEBUG_POTENTIOMETER_1 14
#define DEBUG_POTENTIOMETER_2 15
#define DEBUG_POTENTIOMETER_3 18
#define DEBUG_POTENTIOMETER_2_GND 11
#define DEBUG_POTENTIOMETER_2_VDD 9
#define DEBUG_POTENTIOMETER_3_GND 7
#define DEBUG_POTENTIOMETER_3_VDD 5

#define SERVO_ANGLE_HOLD 0
#define SERVO_ANGLE_FIRE 90

#define UNITID 0x66
#define HWI2C
#define MAXBUFFERSIZE 8

#define DEBUGMODE // Uncomment to enable manual control of motors etc.

#define ERROR_BADCOMMAND_1 1
#define ERROR_BADCOMMAND_2 2
#define ERROR_BADCOMMAND_3 3
#define ERROR_BADCOMMAND_0 4
#define ERROR_I2COVERFLOW  5

/*----------------------------- Global Variables -----------------------------*/

// General
uint8_t errorCode = 0; // If not zero then something bad happened

// Rotation meter
volatile int16_t topCount;
volatile int16_t bottomCount;
volatile int16_t latestBottomRPS;
volatile int16_t latestTopRPS;
#define CALCULATIONINTERVALMICROS 5E5
IntervalTimer frequencyCalculator;

// Servo motor
PWMServo fireServo;

// PID controller
volatile bool doFire = false;
volatile int16_t targetRPS;

/*-------------------------- Prototypes and Macros ---------------------------*/

void handleErrors();
void handleSerial();
void handleDebug();
void evaluateRequests(uint8_t bytes[], uint8_t bytesLength);
void calculateFrequency();
void countTopEdges();
void countBottomEdges();
void evaluatePID();

/*
void requestI2C(void);
void recieveI2C(size_t count);
*/

#define HZTOMICROS(hz) (1000000 / (hz)) // Note, integer math

/*---------------------------------- Main ------------------------------------*/

void setup() {
  Serial.begin(9600);
  /*
  Wire.begin(I2C_SLAVE, UNITID, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.onReceive(receiveI2C);
  Wire.onRequest(requestI2C);
  */
  fireServo.attach(PIN_SERVO);

  pinMode(PIN_LAUNCHERTOP, OUTPUT);
  pinMode(PIN_LAUNCHERBOTTOM, OUTPUT);
  pinMode(PIN_SENSORTOP, INPUT);
  pinMode(PIN_SENSORBOTTOM, INPUT);
  pinMode(PIN_LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_SENSORTOP), countTopEdges, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSORBOTTOM), countBottomEdges, RISING);
  frequencyCalculator.begin(calculateFrequency, CALCULATIONINTERVALMICROS);

  #ifdef DEBUGMODE
  pinMode(DEBUG_POTENTIOMETER_1, INPUT);
  pinMode(DEBUG_POTENTIOMETER_2, INPUT);
  pinMode(DEBUG_POTENTIOMETER_3, INPUT);
  pinMode(DEBUG_POTENTIOMETER_2_GND, OUTPUT);
  pinMode(DEBUG_POTENTIOMETER_2_VDD, OUTPUT);
  digitalWriteFast(DEBUG_POTENTIOMETER_2_GND, LOW);
  digitalWriteFast(DEBUG_POTENTIOMETER_2_VDD, HIGH);
  pinMode(DEBUG_POTENTIOMETER_3_GND, OUTPUT);
  pinMode(DEBUG_POTENTIOMETER_3_VDD, OUTPUT);
  digitalWriteFast(DEBUG_POTENTIOMETER_3_GND, LOW);
  digitalWriteFast(DEBUG_POTENTIOMETER_3_VDD, HIGH);
  #endif
}

void loop() {
  delay(10);
  handleErrors();
  handleSerial();
  #ifdef DEBUGMODE
  handleDebug();
  #else
  evaluatePID();
  #endif
}

void handleErrors() {
  if(errorCode) {
    bool currentLEDState;
    while(1) {
      currentLEDState = !currentLEDState;
      digitalWriteFast(PIN_LED, currentLEDState);
      Serial.println("Error: " + errorCode);
      delay(100);
    }
  }
}

void handleSerial() {
  int currentCount = Serial.available();
  if(currentCount > MAXBUFFERSIZE) {
    errorCode = ERROR_I2COVERFLOW;
    return;
  }
  if(currentCount) {
    uint8_t serialBuffer[currentCount];
    for(int index = 0; index < currentCount; index++) {
      serialBuffer[index] = Serial.read();
    }
    evaluateRequests(serialBuffer, currentCount);
  }
}

void handleDebug() {
  Serial.print("Top:");
  Serial.print(latestBottomRPS);
  Serial.print("Bottom:");
  Serial.print(latestTopRPS);
  fireServo.write((analogRead(DEBUG_POTENTIOMETER_1) * 180) / 1024);
  //analogWrite(PIN_LAUNCHERTOP, analogRead(DEBUG_POTENTIOMETER_2) / 4);
  //analogWrite(PIN_LAUNCHERBOTTOM, analogRead(DEBUG_POTENTIOMETER_3) / 4);
}

void evaluateRequests(uint8_t bytes[], uint8_t bytesLength) {
  if(!bytesLength) return; // We need some data
  switch(bytes[0]) {
    case 'S': // Spin up to desired RPS, RPS is two bytes and original command
      if((bytesLength) != 3) {
        errorCode = ERROR_BADCOMMAND_1;
        return;
      }
      targetRPS = (bytes[1] | bytes[2] >> 8);
      break;
    case 'F': // Fire when ready, only byte is original command
      if((bytesLength) != 1) {
        errorCode = ERROR_BADCOMMAND_2;
        return;
      }
      doFire = true;
      break;
    case 'R': // Prints current RPM to serial, over I2C use requestEvent
      if((bytesLength) != 1) {
        errorCode = ERROR_BADCOMMAND_3;
        return;
      }
      break;
    default: // Unrecognized command
      errorCode = ERROR_BADCOMMAND_0;
      break;
  }
}

/*
void receiveI2C(size_t count) {
  if(count > MAXBUFFERSIZE) {
    errorCode = ERROR_I2COVERFLOW;
    return;
  }
  int8_t buffer[MAXBUFFERSIZE];
  Wire.read(buffer, count);
  evaluateRequests(buffer, count);
}

void requestI2C(void) {
  int16_t valueBuffer[] = {latestBottomRPS, latestTopRPS};
  Wire.write(valueBuffer, sizeof(valueBuffer)); // Prints 4 bytes
}
*/

/*-------------------------------- RPM Meter ---------------------------------*/

void calculateFrequency() {
  cli();
  latestTopRPS =  (topCount * CALCULATIONINTERVALMICROS) / 1E6;
  latestBottomRPS = (bottomCount * CALCULATIONINTERVALMICROS) / 1E6;
  topCount = 0;
  bottomCount = 0;
  sei();
}

void countTopEdges() {
  topCount++;
}

void countBottomEdges() {
  bottomCount++;
}

/*---------------------------------- Servo -----------------------------------*/

/*--------------------------------- PID Loop ---------------------------------*/
