#include <MaxMatrix.h>
#include <avr/pgmspace.h>
#include <Servo.h>
#include "pitches.h"
#include <SPI.h>
#include "RF24.h"
#include <printf.h>

// Set up the NRF24L01
RF24 radio(7,8);
byte addresses[][6] = {"1Node","2Node"};  // Create the pipes

struct dataStruct {
  unsigned long timeCounter;  // Save response times
  char keyPress;          // When a key is pressed, this variable stores its unique code
  int keyState;
  boolean keypadLock;     // When this flag is active, no input will be received fron the keypad
  boolean configMode;     // This flag determines wheter the robot is in Config Mode or not
  boolean statusDizzy;    // Is the robot feeling Dizzy?
} myData;                 // Data stream that will be sent to the robot

// Happy Bithday: Notes in the tune
int melody[] = {
  C4, C4, D4, C4, F4, E4, C4, C4, D4, C4, G4, F4,
  C4, C4, C5, LA4, F4, E4, D4, B4, B4, LA4, F4, G4, F4
};

// Note durations: H = Half note, Q = Quarter note... W = Whole note
int noteDurations[] = {
  Q, Q, Q, Q, Q, H, Q, Q, Q, Q, Q, H,
  Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, Q, H
};
int noteCounter = 0;  // Keeps track of the current note being played
int pauseBetweenNotes = 500; // Creates a pause to differentiate each note
unsigned long melodyPreviousMillis = 0; // Timer that controls when to play the notes

const int buttonA = 18;
const int buttonB = 19;

const int candleA = 14;
const int candleB = 15;
const int candleC = 16;

Servo leftWheel;
Servo rightWheel;

boolean dirLW = true;    // Direction for Left Wheel
boolean dirRW = true;    // Direction for Right Wheel
// True = Forward | False = Reverse 

boolean textEnable = false;
boolean soundEnable = false;
boolean enableLW = false;
boolean enableRW = false;
boolean enableCandleA = false;
boolean enableCandleB = false;
boolean enableCandleC = false;

// Variables to control the robot's animated expressions
unsigned long previousMillis = 0; // Store the last time the Led Matrix is updated
int frameCounter = 0; // Keeps track of the current frame and the maximum number of frames in each animation
int expFrame = 0; // This value sets the sprite for each eye during a frame
int expFrameChange[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // The duration of each frame (in milliseconds)
byte* frameIndex[15]; // This array sets the sequence of sprites that make each animation

// Variables to keep track of the robot's battery level
unsigned long prevBattCheck = 0; //When was the last time that you checked the battery voltage?
float battVoltage[5] = {3.75, 3.75, 3.75, 3.75, 3.75}; //Store 4 samples to average
//The fifth value in the array is made up by averaging the samples
//I have decided to give it some starting values (around 50% of the battery's capacity)
//in order to avoid getting an unusually low (or high!) value at the start of the program
int battChecker = 0;  // This counter arranges the values in battVoltage
int battLevel = 100;  // The battery level, as a percentage
unsigned int checkBattInterval = 100; // Sets up the time between samples
bool firstBattCheck = false; // Battery charge is an important variable for the robot's operation.
//This flag ensures that the robot will perform the checkBattery routine soon after it's turned on

unsigned long prevMotorMillis = 0;
int motorInterval = 500;

// Sprites for the robot's "eyes" 
// Happy blink (800): blinkHEye, happyEye 
byte hppyEyes[] = {8, 8, B00000000, B11111100, B11111110, B00000110, B00000110, B11111110, B11111100, B00000000};
byte blnkEyes[] = {8, 8, B00000000, B01100000, B01110000, B00110000, B00110000, B01110000, B01100000, B00000000};
byte nrmlLEye[] = {8, 8, B00000000, B01111100, B11111110, B11001010, B11111110, B01111100, B00000000, B00000000};
byte nrmlREye[] = {8, 8, B00000000, B00000000, B01111100, B11111110, B11001010, B11111110, B01111100, B00000000};
byte srprLEye[] = {8, 8, B00111100, B01111110, B11000011, B11000011, B11000011, B01111110, B00111100, B00000000};
byte srprREye[] = {8, 8, B00000000, B00111100, B01111110, B11000011, B11000011, B11000011, B01111110, B00111100};
byte clsdEyes[] = {8, 8, B00000000, B00110000, B00110000, B00110000, B00110000, B00110000, B00110000, B00000000};
byte bhrtLEye[] = {8, 8, B00011100, B00100110, B01011110, B11111100, B01111110, B00111110, B00011100, B00000000};
byte bhrtREye[] = {8, 8, B00000000, B00011100, B00111110, B01111110, B11111100, B01011110, B00100110, B00011100};
byte lhrtLEye[] = {8, 8, B00011000, B00111100, B00101100, B01111000, B00111100, B00111100, B00011000, B00000000};
byte lhrtREye[] = {8, 8, B00000000, B00011000, B00111100, B00111100, B01111000, B00101100, B00111100, B00011000};
byte dzzyLEye[] = {8, 8, B00111100, B01100010, B11001001, B11011101, B11010101, B01100100, B00111000, B00000000};
byte dzzyREye[] = {8, 8, B00000000, B00111000, B01100100, B11010101, B11011101, B11001001, B01100010, B00111100};
byte shnyLEye[] = {8, 8, B00111100, B01110110, B11100011, B11110111, B11011111, B01111110, B00111100, B00000000};
byte shnyREye[] = {8, 8, B00000000, B00111100, B01111110, B11011111, B11110111, B11100011, B01110110, B00111100};
byte clsdLids[] = {8, 8, B00000000, B00110000, B01110000, B01000000, B01000000, B01110000, B00110000, B00000000};
byte slpyEyes[] = {8, 8, B00000000, B01111100, B11111100, B11000000, B11000000, B11111100, B01111100, B00000000};
byte cnfgLEye[] = {8, 8, B00000000, B01111110, B01111110, B01000010, B00000000, B01000010, B01111110, B00000000};
byte cnfgREye[] = {8, 8, B00000000, B01111110, B01000010, B00000000, B01000010, B01111110, B01111110, B00000000};
byte lookLeft[] = {8, 8, B01111100, B11001010, B11111110, B11111110, B01111100, B00000000, B00000000, B00000000};
byte lookRigh[] = {8, 8, B00000000, B00000000, B00000000, B01111100, B11111110, B11111110, B11001010, B01111100};
byte sdnsLEye[] = {8, 8, B00001100, B01111100, B11111100, B10011110, B11111110, B01111110, B00000110, B00000000};
byte sdnsREye[] = {8, 8, B00000000, B00000110, B01111110, B11111110, B10011110, B11111100, B01111100, B00001100};
byte angrLEye[] = {8, 8, B00000011, B01111111, B11111111, B11001110, B11111110, B01111100, B00001100, B00000000};
byte angrREye[] = {8, 8, B00000000, B00001100, B01111100, B11111110, B11001110, B11111111, B01111111, B00000011};
byte pstvEyes[] = {8, 8, B00000000, B00011000, B00011100, B00001100, B00001100, B00011100, B00011000, B00000000};
byte ngtvEyeL[] = {8, 8, B00110000, B01110000, B01000000, B01000000, B01110000, B00110000, B00000000, B00000000};
byte ngtvEyeR[] = {8, 8, B00000000, B00000000, B00110000, B01110000, B01000000, B01000000, B01110000, B00110000};

// Character table. Used to display text
PROGMEM const unsigned char CH[] = {
3, 8, B00000000, B00000000, B00000000, B00000000, B00000000, // space
1, 8, B01011111, B00000000, B00000000, B00000000, B00000000, // !
3, 8, B00000011, B00000000, B00000011, B00000000, B00000000, // "
5, 8, B00010100, B00111110, B00010100, B00111110, B00010100, // #
4, 8, B00100100, B01101010, B00101011, B00010010, B00000000, // $
5, 8, B01100011, B00010011, B00001000, B01100100, B01100011, // %
5, 8, B00110110, B01001001, B01010110, B00100000, B01010000, // &
1, 8, B00000011, B00000000, B00000000, B00000000, B00000000, // '
3, 8, B00011100, B00100010, B01000001, B00000000, B00000000, // (
3, 8, B01000001, B00100010, B00011100, B00000000, B00000000, // )
5, 8, B00101000, B00011000, B00001110, B00011000, B00101000, // *
5, 8, B00001000, B00001000, B00111110, B00001000, B00001000, // +
2, 8, B10110000, B01110000, B00000000, B00000000, B00000000, // ,
4, 8, B00001000, B00001000, B00001000, B00001000, B00000000, // -
2, 8, B01100000, B01100000, B00000000, B00000000, B00000000, // .
4, 8, B01100000, B00011000, B00000110, B00000001, B00000000, // /
4, 8, B00111110, B01000001, B01000001, B00111110, B00000000, // 0
3, 8, B01000010, B01111111, B01000000, B00000000, B00000000, // 1
4, 8, B01100010, B01010001, B01001001, B01000110, B00000000, // 2
4, 8, B00100010, B01000001, B01001001, B00110110, B00000000, // 3
4, 8, B00011000, B00010100, B00010010, B01111111, B00000000, // 4
4, 8, B00100111, B01000101, B01000101, B00111001, B00000000, // 5
4, 8, B00111110, B01001001, B01001001, B00110000, B00000000, // 6
4, 8, B01100001, B00010001, B00001001, B00000111, B00000000, // 7
4, 8, B00110110, B01001001, B01001001, B00110110, B00000000, // 8
4, 8, B00000110, B01001001, B01001001, B00111110, B00000000, // 9
2, 8, B01010000, B00000000, B00000000, B00000000, B00000000, // :
2, 8, B10000000, B01010000, B00000000, B00000000, B00000000, // ;
3, 8, B00010000, B00101000, B01000100, B00000000, B00000000, // <
3, 8, B00010100, B00010100, B00010100, B00000000, B00000000, // =
3, 8, B01000100, B00101000, B00010000, B00000000, B00000000, // >
4, 8, B00000010, B01010001, B00001001, B00000110, B00000000, // ?
5, 8, B00111110, B01001001, B01010101, B01011101, B00001110, // @
4, 8, B01111110, B00010001, B00010001, B01111110, B00000000, // A
4, 8, B01111111, B01001001, B01001001, B00110110, B00000000, // B
4, 8, B00111110, B01000001, B01000001, B00100010, B00000000, // C
4, 8, B01111111, B01000001, B01000001, B00111110, B00000000, // D
4, 8, B01111111, B01001001, B01001001, B01000001, B00000000, // E
4, 8, B01111111, B00001001, B00001001, B00000001, B00000000, // F
4, 8, B00111110, B01000001, B01001001, B01111010, B00000000, // G
4, 8, B01111111, B00001000, B00001000, B01111111, B00000000, // H
3, 8, B01000001, B01111111, B01000001, B00000000, B00000000, // I
4, 8, B00110000, B01000000, B01000001, B00111111, B00000000, // J
4, 8, B01111111, B00001000, B00010100, B01100011, B00000000, // K
4, 8, B01111111, B01000000, B01000000, B01000000, B00000000, // L
5, 8, B01111111, B00000010, B00001100, B00000010, B01111111, // M
5, 8, B01111111, B00000100, B00001000, B00010000, B01111111, // N
4, 8, B00111110, B01000001, B01000001, B00111110, B00000000, // O
4, 8, B01111111, B00001001, B00001001, B00000110, B00000000, // P
4, 8, B00111110, B01000001, B01000001, B10111110, B00000000, // Q
4, 8, B01111111, B00001001, B00001001, B01110110, B00000000, // R
4, 8, B01000110, B01001001, B01001001, B00110010, B00000000, // S
5, 8, B00000001, B00000001, B01111111, B00000001, B00000001, // T
4, 8, B00111111, B01000000, B01000000, B00111111, B00000000, // U
5, 8, B00001111, B00110000, B01000000, B00110000, B00001111, // V
5, 8, B00111111, B01000000, B00111000, B01000000, B00111111, // W
5, 8, B01100011, B00010100, B00001000, B00010100, B01100011, // X
5, 8, B00000111, B00001000, B01110000, B00001000, B00000111, // Y
4, 8, B01100001, B01010001, B01001001, B01000111, B00000000, // Z
2, 8, B01111111, B01000001, B00000000, B00000000, B00000000, // [
4, 8, B00000001, B00000110, B00011000, B01100000, B00000000, // \ backslash
2, 8, B01000001, B01111111, B00000000, B00000000, B00000000, // ]
3, 8, B00000010, B00000001, B00000010, B00000000, B00000000, // hat
4, 8, B01000000, B01000000, B01000000, B01000000, B00000000, // _
2, 8, B00000001, B00000010, B00000000, B00000000, B00000000, // `
4, 8, B00100000, B01010100, B01010100, B01111000, B00000000, // a
4, 8, B01111111, B01000100, B01000100, B00111000, B00000000, // b
4, 8, B00111000, B01000100, B01000100, B00101000, B00000000, // c
4, 8, B00111000, B01000100, B01000100, B01111111, B00000000, // d
4, 8, B00111000, B01010100, B01010100, B00011000, B00000000, // e
3, 8, B00000100, B01111110, B00000101, B00000000, B00000000, // f
4, 8, B10011000, B10100100, B10100100, B01111000, B00000000, // g
4, 8, B01111111, B00000100, B00000100, B01111000, B00000000, // h
3, 8, B01000100, B01111101, B01000000, B00000000, B00000000, // i
4, 8, B01000000, B10000000, B10000100, B01111101, B00000000, // j
4, 8, B01111111, B00010000, B00101000, B01000100, B00000000, // k
3, 8, B01000001, B01111111, B01000000, B00000000, B00000000, // l
5, 8, B01111100, B00000100, B01111100, B00000100, B01111000, // m
4, 8, B01111100, B00000100, B00000100, B01111000, B00000000, // n
4, 8, B00111000, B01000100, B01000100, B00111000, B00000000, // o
4, 8, B11111100, B00100100, B00100100, B00011000, B00000000, // p
4, 8, B00011000, B00100100, B00100100, B11111100, B00000000, // q
4, 8, B01111100, B00001000, B00000100, B00000100, B00000000, // r
4, 8, B01001000, B01010100, B01010100, B00100100, B00000000, // s
3, 8, B00000100, B00111111, B01000100, B00000000, B00000000, // t
4, 8, B00111100, B01000000, B01000000, B01111100, B00000000, // u
5, 8, B00011100, B00100000, B01000000, B00100000, B00011100, // v
5, 8, B00111100, B01000000, B00111100, B01000000, B00111100, // w
5, 8, B01000100, B00101000, B00010000, B00101000, B01000100, // x
4, 8, B10011100, B10100000, B10100000, B01111100, B00000000, // y
3, 8, B01100100, B01010100, B01001100, B00000000, B00000000, // z
3, 8, B00001000, B00110110, B01000001, B00000000, B00000000, // {
1, 8, B01111111, B00000000, B00000000, B00000000, B00000000, // |
3, 8, B01000001, B00110110, B00001000, B00000000, B00000000, // }
4, 8, B00001000, B00000100, B00001000, B00000100, B00000000, // ~
1, 8, B01111101, B00000000, B00000000, B00000000, B00000000, // !
4, 8, B00110000, B01001000, B01000101, B00100000, B00000000, // ¿
4, 8, B01111000, B00100110, B00100101, B01111000, B00000000, // Á
4, 8, B01111100, B01010110, B01010101, B01000100, B00000000, // É
3, 8, B01000100, B01111110, B01000101, B00000000, B00000000, // Í
5, 8, B01111100, B00001010, B00010010, B00100001, B01111101, // Ñ
4, 8, B00111000, B01000100, B01000110, B00111001, B00000000, // Ó
4, 8, B00111100, B01000000, B01000010, B00111101, B00000000, // Ú
5, 8, B00100000, B01010100, B01010100, B01111010, B01000001, // á
5, 8, B00111000, B01010100, B01010100, B00011010, B00000001, // é
3, 8, B01001000, B01111010, B01000001, B00000000, B00000000, // í
4, 8, B01111001, B00001001, B00001010, B01110010, B00000000, // ñ
5, 8, B00111000, B01000100, B01000100, B00111010, B00000001, // ó
4, 8, B00111100, B01000000, B01000010, B01111101, B00000000, // ú
};

// Setting up the LED Matrixes using the MAXMATRIX Library
int data = 4;    // DIN pin of MAX7219 module
int load = 5;    // CS pin of MAX7219 module
int clock = 6;  // CLK pin of MAX7219 module
int maxInUse = 2;  //how many MAX7219 are connected
MaxMatrix m(data, load, clock, maxInUse); // define Library

byte buffer[10];

// Scrolling Text
// While the text is being displayed, the mechanical system stops
char string1[] = " Hello World!!!   ";


void setup(){
  //Serial.begin(9600);
  m.init(); // module MAX7219
  m.setIntensity(1); // LED Intensity 0-15
  utf8ascii(string1); // Convert the text string to ASCII values
  m.writeSprite(0, 0, clsdLids); // At startup, the robot will be sleeping
  m.writeSprite(8, 0, clsdLids); // This also masks the momentary "wiggle" of the servos
  
  leftWheel.attach(3);    // Left Wheel on pin 3
  rightWheel.attach(10);  // Right Wheel on pin 10
  // Due to an issue in the servos' control board, there's a "glitch" that makes them
  // move randomly at startup, although there's no digital signal being applied.
  // This issue was solved by adding the following lines.
  // However, this makes the servos "wiggle" for an instant
  delay(100);
  leftWheel.detach();
  rightWheel.detach(); 
  
  pinMode(candleA, OUTPUT);
  pinMode(candleB, OUTPUT);
  pinMode(candleC, OUTPUT);

  radio.begin();  //Initialize NRF24L01
  radio.setDataRate(RF24_250KBPS);  //Data rate is slow, but ensures accuracy
  radio.setPALevel(RF24_PA_HIGH);   //High PA Level, to give enough range
  radio.setCRCLength(RF24_CRC_16);  //CRC at 16 bits
  radio.setRetries(15,15);          //Max number of retries
  radio.setPayloadSize(8);          //Payload size of 8bits

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start listening for data
  radio.startListening();

  // Set the robot's default animation: NormalBlink
  setNormalBlink();
}

void loop(){
  // Start tracking the elapsed time
  unsigned long currentMillis = millis();
  unsigned long melodyMillis = millis();
  unsigned long battCheckMillis = millis();
  

  // Draw the animation's frames to the led matrix  
  if(currentMillis - previousMillis >= expFrameChange[frameCounter])
  {
    if(expFrameChange[frameCounter+1] == 0) // The end frame of a looping animation is defined by setting the next frame change interval to 0
    {
    //If you have reached the last frame, go back to the first one
      frameCounter = 0;
      expFrame = 0;
    }
    if(expFrameChange[frameCounter+1] == 1) // The end frame of an animation that is only played once is defined by setting the next frame change interval to 1
    {
    //After playing the animation, make the robot go back to the default one
      setNormalBlink();
    }
    m.writeSprite(0, 0, frameIndex[expFrame]);
    m.writeSprite(8, 0, frameIndex[expFrame+1]);
  
    previousMillis = currentMillis;  // Remember the time
    // Move to the next frame
    if(expFrameChange[frameCounter+1] > 0) 
    {
      frameCounter++;
      expFrame += 2;
    }
  }

  // Play sounds through a buzzer
  if((melodyMillis - melodyPreviousMillis >= pauseBetweenNotes)&&(noteCounter<25)&&(soundEnable==true))
  // The robot will only play one tune, 
  {
    tone(9, melody[noteCounter], noteDurations[noteCounter]);
    pauseBetweenNotes = noteDurations[noteCounter]*1.30;
    noteCounter++;
    melodyPreviousMillis = melodyMillis;
    if(noteCounter==25) 
    {
      soundEnable = false;
      noteCounter=0;
    }
  }

  // Check battery voltage
  /*
  if(battCheckMillis - prevBattCheck >= checkBattInterval)
  {
    checkBattery();
    prevBattCheck = battCheckMillis;  //Remember the last time the sensor value was checked
    if(battChecker == 4)  //When four samples have been collected
    {
      battChecker = 0;  //Restart from sample #0
      //The following test, which is only performed once per session ensures 
      //that the robot will estimate the battery's charge soon after it's turned on
      if(firstBattCheck == false)
      {
      checkBattInterval = 60000;
      firstBattCheck = true;
      //Afterwards, set up a longer interval for this routine.
      //This way, the robot won't waste processing power and energy
      //constantly checking out his own battery levels
      }
    }
    else
    {
    battChecker++; //If the four samples haven't been collected, increment the counter
    }
  }*/
  
  // Left Wheel control
  if(enableLW==true)
  {
  leftWheel.attach(3);
  if(dirLW == true) leftWheel.write(120);
  if(dirLW == false) leftWheel.write(70);
  }
  if(enableLW == false)
  {
  leftWheel.write(92);
  leftWheel.detach();
  }


  // Right Wheel control
  if(enableRW==true)
  {
  rightWheel.attach(10);
  if(dirRW == true) rightWheel.write(70);
  if(dirRW == false) rightWheel.write(120);
  }
  if(enableRW == false)
  {
  rightWheel.write(93);
  rightWheel.detach();
  }

  // Candlelight control
  if(enableCandleA==true)
  digitalWrite(candleA, HIGH);
  else
  digitalWrite(candleA, LOW);

  if(enableCandleB==true)
  digitalWrite(candleB, HIGH);
  else
  digitalWrite(candleB, LOW);

  if(enableCandleC==true)
  digitalWrite(candleC, HIGH);
  else
  digitalWrite(candleC, LOW);


  // Scrolling text
  if(textEnable == true)
  {
  byte c;
  delay(100);
  m.shiftLeft(false, true);
  printStringWithShift(string1, 100);  // Send scrolling Text
  textEnable = false;
  }

  // Wireless communication
  if ( radio.available())
  {
    while (radio.available())   // While there is data ready to be retrieved from the receive pipe
    {
      radio.read( &myData, sizeof(myData) );             // Get the data
    }

    radio.stopListening();                               // First, stop listening so we can transmit
    radio.write( &myData, sizeof(myData) );              // Send the received data back.
    radio.startListening();                              // Now, resume listening so we catch the next packets.

    //(M)Button: Play the Happy Birthday melody
    if(myData.keyPress == 'M')
    {
      setLaugh();
      soundEnable=true;
    }

    //(A)Button: Show message
    if(myData.keyPress == 'A')
    {
      textEnable=true;
    }

    //(U)Button: Move forward
    if(myData.keyPress == 'U')
    {
      dirRW = true;
      dirLW = true;
      enableRW = true;
      enableLW = true;
    }

    //(D)Button: Move backwards
    if(myData.keyPress == 'D')
    {
      dirRW = false;
      dirLW = false;
      enableRW = true;
      enableLW = true;
    }

    //(L)Button: Move left
    if(myData.keyPress == 'L')
    {
      enableLW = true;
    }

    //(R)Button: Move right
    if(myData.keyPress == 'R')
    {
      enableRW = true;
    }

    
    if(((myData.keyPress == 'U')||(myData.keyPress == 'D')||
    (myData.keyPress == 'L')||(myData.keyPress == 'R'))&&
    (myData.keyState == 2))
    {
      enableRW = false;
      enableLW = false;
    }

  }

}

// Put extracted character on Display
void printCharWithShift(byte c, int shift_speed){
  if (c < 32) return;   // Visible characters in the ASCII table start at position 32, anything below this value isn't a letter or symbol and can't be printed to the dot matrix
  if (c >= 32 && c < 127) c -= 32;  // The character table in the Flash Memory starts at 0, so the ASCII value must be "trimmed"
  // Special characters start at Flash Memory position #95
  if (c == 161) c = 95;  // Print special character ¡
  if (c == 191) c = 96;  // Print special character ¿
  if (c == 193) c = 97;  // Print special character Á
  if (c == 201) c = 98;  // Print special character É
  if (c == 205) c = 99;  // Print special character Í
  if (c == 209) c = 100;  // Print special character Ñ
  if (c == 211) c = 101;  // Print special character Ó
  if (c == 218) c = 102;  // Print special character Ú
  if (c == 225) c = 103;  // Print special character á
  if (c == 233) c = 104;  // Print special character é
  if (c == 237) c = 105;  // Print special character í
  if (c == 241) c = 106;  // Print special character ñ
  if (c == 243) c = 107;  // Print special character ó
  if (c == 250) c = 108;  // Print special character ú
  memcpy_P(buffer, CH + 7*c, 7); // Copy to buffer the contents of the Flash Memory. "CH + 7*c" ensures that it'll read the character data, starting by its column size and row size
  m.writeSprite(maxInUse*8, 0, buffer);
  m.setColumn(maxInUse*8 + buffer[0], 0);
  
  for (int i=0; i<buffer[0]+1; i++) 
  {
    delay(shift_speed);
    m.shiftLeft(false, false);
  }
}

// Extract characters from Scrolling text
void printStringWithShift(char* s, int shift_speed){
  while (*s != 0){
    printCharWithShift(*s, shift_speed);
    s++;
  }
}

// Default animation: The robot has a plain expression, blinks from time to time
void setNormalBlink(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 1000;
  expFrameChange[2] = 200;
  expFrameChange[3] = 3000;
  expFrameChange[4] = 200;
  expFrameChange[5] = 0;
  
  // Set sprites
  frameIndex[0] = nrmlLEye;
  frameIndex[1] = nrmlREye;
  frameIndex[2] = blnkEyes;
  frameIndex[3] = blnkEyes;
  frameIndex[4] = nrmlLEye;
  frameIndex[5] = nrmlREye;
  frameIndex[6] = blnkEyes;
  frameIndex[7] = blnkEyes;
}

// Expression: Laugh
void setLaugh(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 300;
  expFrameChange[2] = 800;
  expFrameChange[3] = 200;
  expFrameChange[4] = 800;
  expFrameChange[5] = 300;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = blnkEyes;
  frameIndex[1] = blnkEyes;
  frameIndex[2] = hppyEyes;
  frameIndex[3] = hppyEyes;
  frameIndex[4] = blnkEyes;
  frameIndex[5] = blnkEyes;
  frameIndex[6] = hppyEyes;
  frameIndex[7] = hppyEyes;
  frameIndex[8] = blnkEyes;
  frameIndex[9] = blnkEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Surprise
void setSurprise(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 100;
  expFrameChange[2] = 800;
  expFrameChange[3] = 200;
  expFrameChange[4] = 800;
  expFrameChange[5] = 200;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = clsdEyes;
  frameIndex[1] = clsdEyes;
  frameIndex[2] = srprLEye;
  frameIndex[3] = srprREye;
  frameIndex[4] = clsdEyes;
  frameIndex[5] = clsdEyes;
  frameIndex[6] = srprLEye;
  frameIndex[7] = srprREye;
  frameIndex[8] = clsdEyes;
  frameIndex[9] = clsdEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: In Love
void setHeartEyes(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 400;
  expFrameChange[2] = 800;
  expFrameChange[3] = 400;
  expFrameChange[4] = 800;
  expFrameChange[5] = 200;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = lhrtLEye;
  frameIndex[1] = lhrtREye;
  frameIndex[2] = bhrtLEye;
  frameIndex[3] = bhrtREye;
  frameIndex[4] = lhrtLEye;
  frameIndex[5] = lhrtREye;
  frameIndex[6] = bhrtLEye;
  frameIndex[7] = bhrtREye;
  frameIndex[8] = blnkEyes;
  frameIndex[9] = blnkEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Dizzy
void setDizzy(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 100;
  expFrameChange[2] = 800;
  expFrameChange[3] = 400;
  expFrameChange[4] = 800;
  expFrameChange[5] = 400;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = clsdEyes;
  frameIndex[1] = clsdEyes;
  frameIndex[2] = dzzyLEye;
  frameIndex[3] = dzzyREye;
  frameIndex[4] = clsdEyes;
  frameIndex[5] = clsdEyes;
  frameIndex[6] = dzzyLEye;
  frameIndex[7] = dzzyREye;
  frameIndex[8] = clsdEyes;
  frameIndex[9] = clsdEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Shiny Eyes
void setShinyEyes(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 100;
  expFrameChange[2] = 1000;
  expFrameChange[3] = 400;
  expFrameChange[4] = 1000;
  expFrameChange[5] = 300;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = clsdLids;
  frameIndex[1] = clsdLids;
  frameIndex[2] = shnyLEye;
  frameIndex[3] = shnyREye;
  frameIndex[4] = clsdLids;
  frameIndex[5] = clsdLids;
  frameIndex[6] = shnyLEye;
  frameIndex[7] = shnyREye;
  frameIndex[8] = clsdLids;
  frameIndex[9] = clsdLids;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Blink with left eye
void setLeftBlink(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 200;
  expFrameChange[2] = 200;
  expFrameChange[3] = 1;

  // Set sprites
  frameIndex[0] = clsdLids;
  frameIndex[1] = nrmlREye;
  frameIndex[2] = nrmlLEye;
  frameIndex[3] = nrmlREye;

  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Blink with right eye
void setRightBlink(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 200;
  expFrameChange[2] = 200;
  expFrameChange[3] = 1;

  // Set sprites
  frameIndex[0] = nrmlLEye;
  frameIndex[1] = clsdLids;
  frameIndex[2] = nrmlLEye;
  frameIndex[3] = nrmlREye;

  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Sadness
void setSadEyes(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 100;
  expFrameChange[2] = 1000;
  expFrameChange[3] = 400;
  expFrameChange[4] = 1000;
  expFrameChange[5] = 300;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = clsdLids;
  frameIndex[1] = clsdLids;
  frameIndex[2] = sdnsLEye;
  frameIndex[3] = sdnsREye;
  frameIndex[4] = clsdLids;
  frameIndex[5] = clsdLids;
  frameIndex[6] = sdnsLEye;
  frameIndex[7] = sdnsREye;
  frameIndex[8] = clsdLids;
  frameIndex[9] = clsdLids;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Anger
void setAngryEyes(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 100;
  expFrameChange[2] = 1000;
  expFrameChange[3] = 200;
  expFrameChange[4] = 1000;
  expFrameChange[5] = 200;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = clsdEyes;
  frameIndex[1] = clsdEyes;
  frameIndex[2] = angrLEye;
  frameIndex[3] = angrREye;
  frameIndex[4] = clsdEyes;
  frameIndex[5] = clsdEyes;
  frameIndex[6] = angrLEye;
  frameIndex[7] = angrREye;
  frameIndex[8] = clsdEyes;
  frameIndex[9] = clsdEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Possitive answer
void setPositive(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 300;
  expFrameChange[2] = 500;
  expFrameChange[3] = 300;
  expFrameChange[4] = 500;
  expFrameChange[5] = 200;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = blnkEyes;
  frameIndex[1] = blnkEyes;
  frameIndex[2] = pstvEyes;
  frameIndex[3] = pstvEyes;
  frameIndex[4] = blnkEyes;
  frameIndex[5] = blnkEyes;
  frameIndex[6] = pstvEyes;
  frameIndex[7] = pstvEyes;
  frameIndex[8] = blnkEyes;
  frameIndex[9] = blnkEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Expression: Negative answer
void setNegative(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 300;
  expFrameChange[2] = 300;
  expFrameChange[3] = 300;
  expFrameChange[4] = 300;
  expFrameChange[5] = 300;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = ngtvEyeL;
  frameIndex[1] = ngtvEyeL;
  frameIndex[2] = ngtvEyeR;
  frameIndex[3] = ngtvEyeR;
  frameIndex[4] = ngtvEyeL;
  frameIndex[5] = ngtvEyeL;
  frameIndex[6] = ngtvEyeR;
  frameIndex[7] = ngtvEyeR;
  frameIndex[8] = ngtvEyeL;
  frameIndex[9] = ngtvEyeL;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Status: Tired - This animation will play when the robot's battery is around 30% - 10%
void setTiredBlink(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 1000;
  expFrameChange[2] = 200;
  expFrameChange[3] = 3000;
  expFrameChange[4] = 200;
  expFrameChange[5] = 0;
  
  // Set sprites
  frameIndex[0] = nrmlLEye;
  frameIndex[1] = nrmlREye;
  frameIndex[2] = clsdLids;
  frameIndex[3] = clsdLids;
  frameIndex[4] = nrmlLEye;
  frameIndex[5] = nrmlREye;
  frameIndex[6] = clsdLids;
  frameIndex[7] = clsdLids;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Status: Sleepy - This animation will play when the robot's battery is 9% or lower
void setSleepyBlink(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 4000;
  expFrameChange[2] = 500;
  expFrameChange[3] = 3000;
  expFrameChange[4] = 500;
  expFrameChange[5] = 0;
  
  // Set sprites
  frameIndex[0] = clsdLids;
  frameIndex[1] = clsdLids;
  frameIndex[2] = slpyEyes;
  frameIndex[3] = slpyEyes;
  frameIndex[4] = clsdLids;
  frameIndex[5] = clsdLids;
  frameIndex[6] = slpyEyes;
  frameIndex[7] = slpyEyes;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Status: Configuration Mode
void setConfigBlink(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 500;
  expFrameChange[2] = 2200;
  expFrameChange[3] = 400;
  expFrameChange[4] = 2000;
  expFrameChange[5] = 0;
  
  // Set sprites
  frameIndex[0] = clsdEyes;
  frameIndex[1] = clsdEyes;
  frameIndex[2] = cnfgLEye;
  frameIndex[3] = cnfgREye;
  frameIndex[4] = clsdEyes;
  frameIndex[5] = clsdEyes;
  frameIndex[6] = cnfgLEye;
  frameIndex[7] = cnfgREye;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Status: Look to the left
void setLeftLook(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 500;
  expFrameChange[3] = 1;

  // Set sprites
  frameIndex[0] = lookLeft;
  frameIndex[1] = lookLeft;

  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Status: Look to the right
void setRightLook(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 500;
  expFrameChange[3] = 1;

  // Set sprites
  frameIndex[0] = lookRigh;
  frameIndex[1] = lookRigh;

  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

// Status: Waking up - This animation will play when the robot is turned on
void setWakingUp(){  
  // Set frame intervals
  expFrameChange[0] = 0;
  expFrameChange[1] = 1000;
  expFrameChange[2] = 400;
  expFrameChange[3] = 800;
  expFrameChange[4] = 200;
  expFrameChange[5] = 1000;
  expFrameChange[6] = 1;
  
  // Set sprites
  frameIndex[0] = slpyEyes;
  frameIndex[1] = slpyEyes;
  frameIndex[2] = clsdLids;
  frameIndex[3] = clsdLids;
  frameIndex[4] = nrmlLEye;
  frameIndex[5] = nrmlREye;
  frameIndex[6] = clsdLids;
  frameIndex[7] = clsdLids;
  frameIndex[8] = nrmlLEye;
  frameIndex[9] = nrmlREye;
  
  // Initialize the animation
  frameCounter = 0;
  expFrame = 0;
}

//Measure the voltage in the LiPo Battery
void checkBattery()
{
  int sensorValue = analogRead(A3); //Read the voltage at pin A3
  battVoltage[battChecker] = sensorValue * (5.00 / 1023.00); //Convert the value to a voltage.
  if(battChecker == 4)  //After taking 4 samples, average the value
  {
    battVoltage[5]=(battVoltage[0]+battVoltage[1]+battVoltage[2]+battVoltage[3]+battVoltage[4])/5;
    Serial.print(">> Average Voltage: ");
    Serial.println(battVoltage[5]);
    int battVoltRound = (battVoltage[5]+0.05)*100; //Round and truncate the value to 2 decimals
    Serial.print(">> Rounded Voltage : ");
    Serial.println(battVoltRound);
    battLevel = ((battVoltRound-350)*10)/7; //Calculate the battery level (as a percentage)
    Serial.print(">> Battery Level: ");
    Serial.println(battLevel);
  }
  
  //Show debugging information
  Serial.print("Value in A3: ");
  Serial.println(sensorValue);
  Serial.print("Voltage Sample #");
  Serial.print(battChecker);
  Serial.print(" : ");
  Serial.println(battVoltage[battChecker]);
  
}

// ****** UTF8-Decoder: convert UTF8-string to extended ASCII *******
static byte c1;  // Last character buffer

// Convert a single Character from UTF8 to Extended ASCII
// Return "0" if a byte has to be ignored
byte utf8ascii(byte ascii) {
    if ( ascii<128 )   // Standard ASCII-set 0..0x7F handling  
    {   c1=0; 
        return( ascii ); 
    }

    // get previous input
    byte last = c1;   // get last char
    c1=ascii;         // remember actual character

    switch (last)     // conversion depending on first UTF8-character
    {   case 0xC2: return  (ascii);  break;
        case 0xC3: return  (ascii | 0xC0);  break;
        case 0x82: if(ascii==0xAC) return(0x80);       // special case Euro-symbol
    }

    return  (0);                                     // otherwise: return zero, if character has to be ignored
}

// convert String object from UTF8 String to Extended ASCII
String utf8ascii(String s)
{  
  String r="";
  char c;
  for (int i=0; i<s.length(); i++)
  {
    c = utf8ascii(s.charAt(i));
    if (c!=0) r+=c;
  }
  return r;
}

// In Place conversion UTF8-string to Extended ASCII (ASCII is shorter!)
void utf8ascii(char* s)
{ 
  int k=0;
  char c;
  for (int i=0; i<strlen(s); i++)
  {
    c = utf8ascii(s[i]);
    if (c!=0) 
      s[k++]=c;
  }
  s[k]=0;
}
