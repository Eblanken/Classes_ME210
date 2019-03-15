#include <Arduino.h>
#include <Metro.h>
#include <IntervalTimer.h>
#include <i2c_t3.h>


uint8_t TestForKey(void);
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

int PIN_SIGNAL_IN = 4;
int LED_PIN = 0;
int PIN_SIGNAL_IN_2 = 5;
int LED_PIN_2 = 1;

//int motor_left = 22;
//int motor_right = 23;

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



  
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  // pinMode(motor_left, OUTPUT);
  // pinMode(motor_right, OUTPUT);
  pinMode(PIN_SIGNAL_IN, INPUT);
  pinMode(PIN_SIGNAL_IN_2, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  frequencyTimer_1.begin(determine_frequency_1, 100000);
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN), count_falling_edges_1, CHANGE);
  frequencyTimer_2.begin(determine_frequency_2, 100000);
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_IN_2), count_falling_edges_2, CHANGE);


}

// Updates frequency of signal depending on potentiometer wiper value
void loop() {

  static int count_print = 1;
  
  if(count_print == 10000) {
    if (freq_1_detected || freq_2_detected) {
      Serial.println("Shoot Now!"); 
      // mem[addr] |= on_mask;
      // if( (reported_duty_cycle_1 <= 35 )|| 
      //   (reported_duty_cycle_2 <= 35) ){
      //   mem[addr] |= dragon_mask;
      // }

  } else {
      Serial.println("No Action Needed");
     // mem[addr] = 0x00;
    }
    count_print = 0;
  }

 


  // if(up_down == 'u') {
  //   digitalWrite(motor_left, HIGH);
  //   digitalWrite(motor_right, LOW);
  // }
  // else if(up_down == 'd') {
  //   digitalWrite(motor_left, LOW);
  //   digitalWrite(motor_right, HIGH);

  // }else {
  //   digitalWrite(motor_left, LOW);
  //   digitalWrite(motor_right, LOW);

  // }

  if (freq_1_detected) {
    digitalWrite(LED_PIN, HIGH);
    

  } else {
    digitalWrite(LED_PIN, LOW);
  }

  if (freq_2_detected) {
    digitalWrite(LED_PIN_2, HIGH);
  

  } else {
    digitalWrite(LED_PIN_2, LOW);
  }

  count_print = count_print + 1;
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