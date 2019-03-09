#include <Arduino.h>
#include <Servo.h>

#define PIN_LAUNCHERTOP     22
#define PIN_LAUNCHERBOTTOM  23


#define FIRE_INPUT 16

#define SERVO_PIN 3

Servo myservo;

bool value = false;





void setup() {
  Serial.begin(9600);
  pinMode(PIN_LAUNCHERTOP, OUTPUT);
  pinMode(PIN_LAUNCHERBOTTOM, OUTPUT);
  pinMode(FIRE_INPUT, INPUT_PULLDOWN);
  //pinMode(SERVO_PIN, OUTPUT);
  myservo.attach(SERVO_PIN);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  myservo.write(0);

}

void loop() {

  static int count_print = 1;
  if(count_print == 1000) {
    count_print = 1;
    Serial.println(value);
  }
  count_print = count_print+1;
  value = digitalRead(FIRE_INPUT);

  if(value) {
      analogWrite(PIN_LAUNCHERTOP, 200);
      analogWrite(PIN_LAUNCHERBOTTOM, 200);
      delay(2000);
      myservo.write(180);

  }
  else {
    analogWrite(PIN_LAUNCHERTOP, 0);
    analogWrite(PIN_LAUNCHERBOTTOM, 0);
    myservo.write(0);
  }


// myservo.write(0);
// delay(2000);
// myservo.write(90);
// delay(2000);


  

  
}