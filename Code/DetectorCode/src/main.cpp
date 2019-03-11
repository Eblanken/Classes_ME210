#include <Arduino.h>
#include <Metro.h>
#include <IntervalTimer.h>
#include <i2c_t3.h>


uint8_t TestForKey(void);
void handleMotorEvents();
void handleLEDEvents();
void RespToKey(void);
void count_falling_edges_1(void);
void determine_frequency_1(void);
void count_falling_edges_2(void);
void determine_frequency_2(void);
void requestEvent(void);
void receiveEvent(size_t len);


int count_1 = 0;
int count_2 = 0;

unsigned long rising_edge_1_start = 0;
unsigned long time_high_1 = 0;
unsigned long rising_edge_2_start = 0;
unsigned long time_high_2 = 0;

unsigned long avg_duty_cycle_1 = 0;
unsigned long avg_duty_cycle_2 = 0;


bool freq_1_detected = false;
bool freq_2_detected = false;

unsigned long reported_duty_cycle_1 = 0;
unsigned long reported_duty_cycle_2 = 0;

IntervalTimer frequencyTimer_1;
IntervalTimer frequencyTimer_2;

#define PIN_SIGNAL_IN   4
#define LED_PIN_1       0
#define PIN_SIGNAL_IN_2 5
#define LED_PIN_2       1
#define OUTPUT_TO_MASTER 18

#define PIN_MOTOR_1     22
#define PIN_MOTOR_2     23
#define PIN_LIMITSWITCH 13
//#define TIME_ROUND      (60 * 2 + 10) * 1E3 // Total round time (ms)
#define TIME_ROUND      ((60 * 2 + 10) * 1E3)
#define TIME_RAISE      (10 * 1E3)            // Time required to raise/lower tower
#define TIME_SPARE      5                     // Extra time to retract just in case

char transmit_byte = 0x00;
char on_mask = 0x01;
char dragon_mask = 0x02;

#define WRITE    0x10
#define READ     0x20




#define MEM_LEN 1
uint8_t mem[MEM_LEN];
uint8_t cmd;
size_t addr;
size_t up_down;


// Configures Pin and Interrupts
void setup() {
  Serial.begin(9600);

  // Wire.begin(I2C_SLAVE, 0x66, I2C_PINS_16_17, I2C_PULLUP_EXT);

  // addr = 0;
  // cmd = 0;
  // memset(mem, 0, sizeof(mem));

  // Wire.onRequest(requestEvent);
  // Wire.onReceive(receiveEvent);


  pinMode(PIN_LIMITSWITCH, INPUT_PULLUP);
  pinMode(PIN_MOTOR_1, OUTPUT);
  pinMode(PIN_MOTOR_2, OUTPUT);

  pinMode(OUTPUT_TO_MASTER, OUTPUT);
  digitalWrite(OUTPUT_TO_MASTER, LOW);

  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(PIN_SIGNAL_IN, INPUT);
  pinMode(PIN_SIGNAL_IN_2, INPUT);

  frequencyTimer_1.begin(determine_frequency_1, 100000);
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN), count_falling_edges_1, CHANGE);
  frequencyTimer_2.begin(determine_frequency_2, 100000);
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN_2), count_falling_edges_2, CHANGE);


}

// Updates frequency of signal depending on potentiometer wiper value
void loop() {
  handleMotorEvents();
  handleLEDEvents();
}

// Raises the tower at the beginning of the match, lowers it at the end
void handleMotorEvents() {
  bool hasTurnedOn = false;
  uint32_t currentTime = millis();
    
  // Measures limit switch
  static uint16_t switchState = 0;
  switchState = ((switchState << 1) | digitalRead(PIN_LIMITSWITCH));

  // Makes sure tower is initially down
  static bool isDown = false;
  if(!isDown) {
    if(switchState == 0xFFFF) {
      isDown = true;
    } else {
      digitalWrite(PIN_MOTOR_1, LOW);
      digitalWrite(PIN_MOTOR_2, HIGH);
      hasTurnedOn = true;
    }
  }

  // Raises tower at beginning
  if(currentTime < TIME_RAISE && !hasTurnedOn) {
    digitalWrite(PIN_MOTOR_1, HIGH);
    digitalWrite(PIN_MOTOR_2, LOW);
    hasTurnedOn = true;
  }

  // Lowers tower
  if((currentTime > (TIME_ROUND - (TIME_SPARE + TIME_RAISE))) && (currentTime < TIME_ROUND) && (switchState != 0xFFFF) && !hasTurnedOn) {
    digitalWrite(PIN_MOTOR_1, LOW);
    digitalWrite(PIN_MOTOR_2, HIGH);
    hasTurnedOn = true;
  }

  // Clears motor if not supposed to be actuating
  if(!hasTurnedOn) {
      digitalWrite(PIN_MOTOR_1, LOW);
      digitalWrite(PIN_MOTOR_2, LOW);
  }
}

void handleLEDEvents() {
  if (freq_1_detected) {
    digitalWrite(LED_PIN_1, HIGH);
  } else {
    digitalWrite(LED_PIN_1, LOW);
  }
  if (freq_2_detected) {
    digitalWrite(LED_PIN_2, HIGH);
  } else {
    digitalWrite(LED_PIN_2, LOW);
  }

  if(freq_1_detected || freq_2_detected) {
    digitalWrite(OUTPUT_TO_MASTER, HIGH);
  } else {
    digitalWrite(OUTPUT_TO_MASTER, LOW);
  }
}

void receiveEvent(size_t len) {
   up_down = Wire.readByte();

}

void requestEvent(void) {
    Wire.write(&mem[addr], MEM_LEN-addr);
}

// Counts edges by incrementing on each call
void count_falling_edges_1(void) {

  bool value = digitalRead(PIN_SIGNAL_IN);
  if(value == 0) {
    //falling edge
    count_1 = count_1+1;
    unsigned long curr_time = micros();
    time_high_1 = curr_time - rising_edge_1_start;
    avg_duty_cycle_1 = avg_duty_cycle_1 + time_high_1;
  }
  else {
    //rising edge
    rising_edge_1_start = micros();
  }



}

void count_falling_edges_2(void) {

  bool value = digitalRead(PIN_SIGNAL_IN_2);
  if(value == 0) {
    //falling edge
    count_2 = count_2+1;
    unsigned long curr_time = micros();
    time_high_2 = curr_time - rising_edge_2_start;
    avg_duty_cycle_2 = avg_duty_cycle_2 + time_high_2;

  }
  else {
    //rising edge
    rising_edge_2_start = micros();
  }


}


// Calls Interrupt to Determine Number of Edges
void determine_frequency_1(void) {
  detachInterrupt(PIN_SIGNAL_IN);
  if (count_1 >= 80 && count_1 <= 200) {
    freq_1_detected = true;
    Serial.print("freq_1 = ");
    Serial.println(10*count_1);
    reported_duty_cycle_1 = (100*avg_duty_cycle_1)/100000;
    Serial.print("duty_cycle_1 = ");
    Serial.println(reported_duty_cycle_1);
  } else {
    freq_1_detected = false;
  }
  avg_duty_cycle_1 = 0;
  count_1 = 0;
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN), count_falling_edges_1, CHANGE);
}

// Calls Interrupt to Determine Number of Edges
void determine_frequency_2(void) {
  detachInterrupt(PIN_SIGNAL_IN_2);
  if (count_2 >= 80 && count_2 <= 200) {
    freq_2_detected = true;
    Serial.print("freq_2 = ");
    Serial.println(10*count_2);
    reported_duty_cycle_2 = (100*avg_duty_cycle_2)/100000;
    Serial.print("duty_cycle_2 = ");
    Serial.println(reported_duty_cycle_2);
  } else {
    freq_2_detected = false;

  }
  avg_duty_cycle_2 = 0;
  count_2 = 0;
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN_2), count_falling_edges_2, CHANGE);
}

uint8_t TestForKey(void) {
  uint8_t KeyEventOccurred;
  KeyEventOccurred = Serial.available();
  return KeyEventOccurred;
}

void RespToKey(void) {
//      uint8_t theKey;
 //     theKey = Serial.read();
}
