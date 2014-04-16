/* 
 * ////////////////////////////////////////////////////
 * /////Built from many people and their hard work/////
 * ////////////////////////////////////////////////////
 * Godzilla Room PIR Sensor attached to a Power Switch tail, 
 * detects movement with the room, turns on the lights,
 * tweets to https://twitter.com/T1GodzillaRoom
 * Uses Arduino UNO, Ethernet Shield, and Power Switch Tail.
 * ////////////////////////////////////////////////////
 * //making sense of the Parallax PIR sensor's output//
 * ////////////////////////////////////////////////////
 *
 * Switches a LED according to the state of the sensors output pin.
 * Determines the beginning and end of continuous motion sequences.
 *
 * @author: Kristian Gohlke / krigoo (_) gmail (_) com / http://krx.at
 * @date:   3. September 2006 
 *
 * kr1 (cleft) 2006 
 * released under a creative commons "Attribution-NonCommercial-ShareAlike 2.0" license
 * http://creativecommons.org/licenses/by-nc-sa/2.0/de/
 *
 *
 * The Parallax PIR Sensor is an easy to use digital infrared motion sensor module. 
 * (http://www.parallax.com/detail.asp?product_id=555-28027)
 *
 * The sensor's output pin goes to HIGH if motion is present.
 * However, even if motion is present it goes to LOW from time to time, 
 * which might give the impression no motion is present. 
 * This program deals with this issue by ignoring LOW-phases shorter than a given time, 
 * assuming continuous motion is present during these phases.
 *  
 */
 /////PUSHINGBOX Setup/////
#include <SPI.h>
#include <Ethernet.h>
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x19 };   // Be sure this address is unique in your network
//Your secret DevID from PushingBox.com. You can use multiple DevID  on multiple Pin if you want
#define DEVID1 "v6B989832F43952A"        //Scenario : "Lights on Baby!"
#define DEVID2 "v98C2317AB7D9236"        //Scenario : "Lights out Baby!"
// Debug mode
#define DEBUG true
char serverName[] = "api.pushingbox.com";
boolean pirPinState = false;
EthernetClient client;
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 10;        
//the time when the sensor outputs a low impulse
long unsigned int lowIn;         
//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 900000;///15 mins  
boolean lockLow = true;
boolean takeLowTime;  

int pirPin = 7;    //the digital pin connected to the PIR sensor's output
int powertail = 6; //Power Switch Tail for + connection
int ledPin = 13; //debug LED

///SETUP///

void setup(){
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(powertail, OUTPUT);
  digitalWrite(pirPin, LOW);
 if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    while(true);
  }
  else{
    Serial.println("Ethernet ready");
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  //give the sensor some time to calibrate
    Serial.print("calibrating sensor ");
      for(int i = 0; i < calibrationTime; i++){
        Serial.print(".");
        delay(1000);
        }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
  }
  
///LOOP///

void loop(){

     if(digitalRead(pirPin) == HIGH){
       digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
        //Sending request to PushingBox when the pin is HIGH
       if(lockLow){  
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;
         digitalWrite(6, HIGH); // Turn the Powertail on
         Serial.println("---");
         Serial.println("Switch ON");
         Serial.print("motion detected at ");
         Serial.print(millis()/1000);
         Serial.println(" sec"); 
        sendToPushingBox(DEVID1); //Send message to Pushingbox to Tweet about it
         delay(50);
         }         
         takeLowTime = true;
       }

     if(digitalRead(pirPin) == LOW){       
       digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state

       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause, 
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){  
           //makes sure this block of code is only executed again after 
           //a new motion sequence has been detected
           lockLow = true;
           digitalWrite(6, LOW);           
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause)/1000);
           Serial.println(" sec");
         digitalWrite(6, LOW); // Turn the Powertail on
        sendToPushingBox(DEVID2);//Send message to Pushingbox to Tweet about it
           
           delay(50);
           }
       }
             // Listening for the pirPin state
      ////
      if (digitalRead(pirPin) == HIGH && pirPinState == false) // switch on pirPin is ON 
      {
        if(DEBUG){Serial.println("pirPin is HIGH");}
        pirPinState = true;
        //Sending request to PushingBox when the pin is HIGHT

      }
       if (digitalRead(pirPin) == LOW && pirPinState == true) // switch on pirPin is OFF
      {
        if(DEBUG){Serial.println("pirPin is LOW");}
        pirPinState = false;
        //Sending request to PushingBox when the pin is LOW
        //sendToPushingBox(DEVID1);
              

      }
  }
  //Function for sending the request to PushingBox
void sendToPushingBox(String devid){
    if(DEBUG){Serial.println("connecting...");}

  if (client.connect(serverName, 80)) {
    if(DEBUG){Serial.println("connected");}

    if(DEBUG){Serial.println("sendind request");}
    client.print("GET /pushingbox?devid=");
    client.print(devid);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverName);
    client.println("User-Agent: Arduino");
    client.println();
  } 
  else {
    if(DEBUG){Serial.println("connection failed");}
  }
  
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if(DEBUG){
    if (client.available()) {
    char c = client.read();
    Serial.print(c);
    }
  }

    if(DEBUG){Serial.println();}
    if(DEBUG){Serial.println("disconnecting.");}
    client.stop();
}