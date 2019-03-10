#include <Arduino.h>
#include <Servo.h>

#define PIN_LAUNCHERTOP     22
#define PIN_LAUNCHERBOTTOM  23


#define START_MOTOR_IN 16

#define RELEASE_SERVO_IN 17

#define SERVO_PIN 3

Servo myservo;

bool value = false;





void setup() {
  Serial.begin(9600);
  pinMode(PIN_LAUNCHERTOP, OUTPUT);
  pinMode(PIN_LAUNCHERBOTTOM, OUTPUT);

  pinMode(START_MOTOR_IN, INPUT_PULLDOWN);
  pinMode(RELEASE_SERVO_IN, INPUT_PULLDOWN);

  myservo.attach(SERVO_PIN);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  myservo.write(0);

}

void loop() {

  static int count_print = 1;
  static bool motor_value = false;
  static bool servo_value = false;
  if(count_print == 1000) {
    count_print = 1;
    Serial.println(motor_value);
    Serial.println(servo_value);
  }
  count_print = count_print+1;
  motor_value = digitalRead(START_MOTOR_IN);
  servo_value = digitalRead(RELEASE_SERVO_IN);

  if(motor_value) {
      analogWrite(PIN_LAUNCHERTOP, 200);
      analogWrite(PIN_LAUNCHERBOTTOM, 200);

  }
  else {
    analogWrite(PIN_LAUNCHERTOP, 0);
    analogWrite(PIN_LAUNCHERBOTTOM, 0);
  }

  if(servo_value) {
       myservo.write(180);
       delay(3000);
       myservo.write(0);
  }
 
     



  

  
}