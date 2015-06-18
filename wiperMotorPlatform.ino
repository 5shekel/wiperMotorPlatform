

#include "Streaming.h"
#include <SPI.h>
#include "RF24.h"

#define DEBUG 1

byte addresses[][7] = {"ampBot","ampCtl"};

typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
role_e role ;      

/**
* Create a data structure for transmitting and receiving data
* This allows many variables to be easily sent and received in a single transmission
* See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct{
  unsigned long _micros;
  int xin;
  int yin;
}myData;


RF24 radio(9,8); //also miso/mosi at 11/12/13
#define role_pin 2
///////  remote /////////////
#define AX_pin A0
#define AY_pin A1
///////// motor /////////////
int inApin[2] = {A3, 6};  // INA: Clockwise input
//had to change inBpin from the default  8,9 
//beacuse of conflict with NRF24 wiring // on the nano side :/
int inBpin[2] = {7, 10}; // INB: Counter-clockwise input
int pwmpin[2] = {3, 5}; // PWM input

int x2pwm, y2pwm;
int leftMotorSpeed, rightMotorSpeed;

void setup() {
  Serial.begin(115200);
  radio.begin();  

  pinMode(role_pin, INPUT);
  digitalWrite(role_pin, HIGH);
  delay(20); // Just to get a solid reading on the role pin
  // read the address pin, establish our role
  if ( digitalRead(role_pin) ) {
    role = role_pong_back;

    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
    radio.startListening();
  
    Serial << "im a bot!" << endl;
  } else {
    role = role_ping_out;

    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
    radio.stopListening();

    Serial << "im a controller!" << endl;
  }

  if(role==role_pong_back)
  {
      // Initialize motor pins as outputs
      for (int i = 0; i < 2; i++)
      {
        pinMode(inApin[i], OUTPUT);
        pinMode(inBpin[i], OUTPUT);
        pinMode(pwmpin[i], OUTPUT);
      }
      // Initialize braked
      for (int i = 0; i < 2; i++)
      {
        digitalWrite(inApin[i], LOW);
        digitalWrite(inBpin[i], LOW);
      }
    }else if(role == role_ping_out){
      pinMode(AX_pin, INPUT);
      pinMode(AY_pin, INPUT);
    }
}


void loop() {
/****************** Ping Out Role ***************************/  

  if (role == role_ping_out)  {
    myData.xin = analogRead(AX_pin);
    myData.yin = analogRead(AY_pin);

    bool ok = radio.write( &myData, sizeof(myData) );
    if (ok)
      Serial << "transfer OK  \n\r";
    else
      Serial << "transfer failed \n\r";
  }



/****************** Pong Back Role ***************************/

  else if ( role == role_pong_back )
  {
    
    if( radio.available()){
                                                           // Variable for the received timestamp
      while (radio.available()) {                          // While there is data ready
        radio.read( &myData, sizeof(myData) );             // Get the payload
      }
      //process data
      x2pwm = map(myData.xin, 0, 1024, 255, -255);
      y2pwm = map(myData.yin, 0, 1024, 255, -255);

      leftMotorSpeed = constrain(y2pwm + x2pwm, -255, 255);
      rightMotorSpeed = constrain(y2pwm - x2pwm, -255, 255);

      if (myData.xin == 518) 
        stop(0);
      else
        move(0, leftMotorSpeed); 
      
      if (myData.yin == 521)
        stop(1);
      else
        move(1, rightMotorSpeed);

      if(DEBUG) Serial<<"left:"<<leftMotorSpeed<<"  right:"<<rightMotorSpeed<<endl; delay(4);
      if(DEBUG) Serial<<"x:"<<myData.xin<<"  y:"<<myData.yin<<endl; delay(4);
   }
 }


  delay(100); //pause comm, tweak!
} // Loop


// motor functions
void move(int motor, int speed) {
      if (speed < 0)
        {
        digitalWrite(inApin[motor], HIGH);
        digitalWrite(inBpin[motor], LOW); 
        }
      else{ 
        digitalWrite(inApin[motor], LOW);
        digitalWrite(inBpin[motor], HIGH);
      }

      analogWrite(pwmpin[motor], abs(speed));
}


void stop(int motor) {
  digitalWrite(inApin[motor], LOW);
  digitalWrite(inBpin[motor], LOW);
  analogWrite(pwmpin[motor], 0);

}


