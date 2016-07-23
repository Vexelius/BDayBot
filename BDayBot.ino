#include <MaxMatrix.h>
#include <avr/pgmspace.h>
#include <Servo.h> 

const int buttonA = 2;
const int buttonB = 4;
const int buttonC = 5;
const int buttonD = 6;

Servo leftWheel;
Servo rightWheel;

boolean dirLW = true;    // Direction for Left Wheel
boolean dirRW = true;    // Direction for Right Wheel
// True = Forward | False = Reverse 

boolean textEnable = false;
boolean spriteTst = true;

// Variables to control the robot's animated expressions
unsigned long previousMillis = 0; //Store the last time the Led Matrix is updated
int frameCounter = 0; // Keeps track of the current frame and the maximum number of frames in each animation
int expFrame = 0; // This value sets the sprite for each eye during a frame
int expFrameChange[10] = {1000, 500, 800, 300, 900, 400, 1000, 0}; // The duration of each frame (in milliseconds)
byte* frameIndex[15]; // This array sets the sequence of sprites that make each animation

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
byte upprLEye[] = {8, 8, B00000000, B00000000, B01111100, B11110010, B11111110, B01111100, B00000000, B00000000}; 

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
int data = 8;    // DIN pin of MAX7219 module
int load = 9;    // CS pin of MAX7219 module
int clock = 10;  // CLK pin of MAX7219 module
int maxInUse = 2;  //how many MAX7219 are connected
MaxMatrix m(data, load, clock, maxInUse); // define Library

byte buffer[10];

// Scrolling Text
// While the text is being displayed, the mechanical system stops
char string1[] = " !Feliz cumpleaños, Ángel!   ";


void setup(){
  leftWheel.attach(3);    //  Left Wheel on pin 3
  rightWheel.attach(11);  // Right Wheel on pin 11 
  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);
  pinMode(buttonC, INPUT);
  pinMode(buttonD, INPUT);
  
  m.init(); // module MAX7219
  m.setIntensity(1); // LED Intensity 0-15
  utf8ascii(string1); // Convert the text string to ASCII values

  // Blink: The robot's default animation
  frameIndex[0] = nrmlLEye;
  frameIndex[1] = nrmlREye;
  frameIndex[2] = blnkEyes;
  frameIndex[3] = blnkEyes;
  frameIndex[4] = hppyEyes;
  frameIndex[5] = hppyEyes;
  frameIndex[6] = dzzyLEye;
  frameIndex[7] = dzzyREye;
  frameIndex[8] = clsdEyes;
  frameIndex[9] = clsdEyes;
  frameIndex[10] = srprLEye;
  frameIndex[11] = srprREye;
  frameIndex[12] = shnyLEye;
  frameIndex[13] = shnyREye;
}

void loop(){
  // Start tracking the elapsed time
  unsigned long currentMillis = millis();

  // Draw the animation's frames to the led matrix
  if(currentMillis - previousMillis >= expFrameChange[frameCounter])
  {
    m.writeSprite(0, 0, frameIndex[expFrame]);
    m.writeSprite(8, 0, frameIndex[expFrame+1]);
  
    previousMillis = currentMillis;  // Remember the time
    if(expFrameChange[frameCounter+1] > 0) 
    {
      frameCounter++;
      expFrame += 2;
    }
    else 
    {
      frameCounter = 0;
      expFrame = 0;
    }
  }
  
  
  if(digitalRead(buttonA)==HIGH)
  {
  leftWheel.write(0);
  if(dirLW == true)
    {
    leftWheel.write(180);
    }
  else
    {
    leftWheel.write(0);
    }
  dirLW = !dirLW;
  }
if(digitalRead(buttonB)==HIGH)
  {
  leftWheel.write(93);
  //One of the DGS04NF has its Stop point at 93, the other at 92
  }


if(digitalRead(buttonC)==HIGH)
  {
  rightWheel.write(0);
  if(dirLW == true)
    {
    rightWheel.write(180);
    }
  else
    {
    rightWheel.write(0);
    }
  dirLW = !dirLW;
  }
if(digitalRead(buttonD)==HIGH)
  {
  rightWheel.write(93);
  }

  if(textEnable == true)
  {
  byte c;
  delay(100);
  m.shiftLeft(false, true);
  printStringWithShift(string1, 100);  // Send scrolling Text
  textEnable = false;
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
