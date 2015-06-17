

#include "Streaming.h"
#include <SPI.h>
#include "RF24.h"

byte addresses[][6] = {"1Node","2Node"};
RF24 radio(9,8);

#define role_pin 2
typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
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
  boolean btn;
}myData;


///////  remote /////////////
#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce-Arduino-Wiring
Bounce bouncer = Bounce();
uint8_t snsVal[] = {0, 0, 0, 0, 0}; //int>byte array (hi and low for anlog and one byte for btn) AX,AY,btn;
int xout, yout, btnout;
int x2pwm, y2pwm ;
int leftMotorSpeed, rightMotorSpeed;
#define BTN_pin 7
#define AX_pin A0
#define AY_pin A1

/// motor shield init
#define BRAKEVCC 0
#define CW 1
#define CCW 2
#define BRAKEGND 3
#define CS_THRESHOLD 100
int inApin[2] = {7, 4};  // INA: Clockwise input
int inBpin[2] = {10, 11}; // INB: Counter-clockwise input
int pwmpin[2] = {5, 6}; // PWM input

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
    pinMode(BTN_pin, INPUT);
    digitalWrite(BTN_pin, HIGH);
    // Bounce object with a 20 millisecond debounce time

    bouncer.attach(BTN_pin);
    bouncer.interval(20);
    }

}


void loop() {
/****************** Ping Out Role ***************************/  

  if (role == role_ping_out)  {
    myData.xin = analogRead(AX_pin);
    myData.yin = analogRead(AY_pin);
    myData.btn = bouncer.read();

     if (radio.write( &myData, sizeof(myData) )){
       //Serial.println(F("all good"));
     }
  }



/****************** Pong Back Role ***************************/

  if ( role == role_pong_back )
  {
    
    if( radio.available()){
                                                           // Variable for the received timestamp
      while (radio.available()) {                          // While there is data ready
        radio.read( &myData, sizeof(myData) );             // Get the payload
      }
     
      //radio.stopListening();                               // First, stop listening so we can talk  
      //myData.value += 0.01;                                // Increment the float value
      //radio.write( &myData, sizeof(myData) );              // Send the final one back.      
      //radio.startListening();                              // Now, resume listening so we catch the next packets.     
      Serial<<"x:"<<myData.xin<<"  y:"<<myData.yin<<"  btn:"<<myData.btn<<endl;
   }
 }



} // Loop
