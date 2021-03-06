// Codeschloss Tobias Kuch 2020 GPL 3.0
#include <Keypad.h> 
#include <SoftwareSerial.h> 

#define RGBLED_R 11 
#define RGBLED_G 10
#define RGBLED_B 9 
#define RGBFadeInterval1  10       // in ms
#define KeybModeTimeInterval1 5000 // in ms
#define PIEZOSUMMER A1
#define CyclesInBlackMax 20

#define RGBOFF 0
#define RGBSHORTBLACK 8
#define RGBRED 1
#define RGBGREEN 2
#define RGBBLUE 3
#define RGBWHITE 4
#define RGBYELLOW 5
#define RGBCYAN 6
#define RGBMAGENTA 7


const byte ROWS = 4; 
const byte COLS = 4; 
const byte MaxPinCodeLength = 20;

SoftwareSerial mySerial(12, 13); // RX, TX
 
char keys[ROWS][COLS] = { 
                          {1,2,3,13},  
                          {4,5,6,14}, 
                          {7,8,9,15},  
                          {10,11,12,16},
                         }; 
byte colPins[COLS] = {A0,8,7,6}; //A0,8,7,6;
byte rowPins[ROWS]= {5,4,3,2}; // 5,4,3,2}

byte RGBValue_R = 0;
byte RGBValue_G = 0;
byte RGBValue_B = 0;
byte RGBFadeValue_R = 0;
byte RGBFadeValue_G = 0;
byte RGBFadeValue_B = 0;
bool RGBFadeDir_R = true;
bool RGBFadeDir_G = true;
bool RGBFadeDir_B = true;


byte key = 0;
bool InSync = true;
bool CodeEnterSeqence = false;
bool CodeEnterSeqenceOLD = false;
bool InputBlocked = false;
bool PinEnteredFalseBefore  = false;
bool RGBFadeEnabled = true;

long previousMillis = 0;
long previousMillisKeyBoard = 0;   
byte EnCodedKeyStroke = 0;
byte inByte = 0;
int CyclesInBlack = 0;

unsigned long InititalKey = 902841;


Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 


void setup()
{  
 mySerial.begin(9600); 
 Serial.begin(9600); 
 pinMode(RGBLED_G,OUTPUT); // Ausgang RGB LED Grün
 pinMode(RGBLED_R,OUTPUT); // Ausgang RGB LED Rot
 pinMode(RGBLED_B,OUTPUT); // Ausgang RGB LED Blau
 pinMode(PIEZOSUMMER,OUTPUT); // Ausgang RGB LED Blau
 digitalWrite(PIEZOSUMMER,LOW); // Ausgang RGB LED Blau
 RGBControl(RGBWHITE,false); // NORMAL MODE

 randomSeed(InititalKey);
 RGBControl(RGBBLUE,true); // NORMAL MODE
}

void RGBControl(byte function, bool fadeit)
{
 if (function == RGBOFF)
  {
  RGBValue_R = 0;
  RGBValue_G = 0;
  RGBValue_B = 0;
  RGBFadeValue_R = 0;
  RGBFadeValue_G = 0;
  RGBFadeValue_B = 0;
  RGBFadeDir_R = true;
  RGBFadeDir_G = true;
  RGBFadeDir_B = true; 
  }
 if (function == RGBRED)
  {
  RGBValue_R = 255;
  RGBValue_G = 0;
  RGBValue_B = 0;
  RGBFadeValue_R = 255;
  RGBFadeValue_G = 0;
  RGBFadeValue_B = 0;
  RGBFadeDir_R = false;
  RGBFadeDir_G = true;
  RGBFadeDir_B = true; 
  }
 if (function == RGBGREEN)
  {
  RGBValue_R = 0;
  RGBValue_G = 255;
  RGBValue_B = 0;
  RGBFadeValue_R = 0;
  RGBFadeValue_G = 255;
  RGBFadeValue_B = 0;
  RGBFadeDir_R = true;
  RGBFadeDir_G = false;
  RGBFadeDir_B = true;
  }
 if (function ==  RGBBLUE)
  {
  RGBValue_R = 0;
  RGBValue_G = 0;
  RGBValue_B = 255;
  RGBFadeValue_R = 0;
  RGBFadeValue_G = 0;
  RGBFadeValue_B = 255;
  RGBFadeDir_R = true;
  RGBFadeDir_G = true;
  RGBFadeDir_B = false;
  }
 if (function == RGBWHITE)
  {
  RGBValue_R = 255;
  RGBValue_G = 255;
  RGBValue_B = 255;
  RGBFadeValue_R = 255;
  RGBFadeValue_G = 255;
  RGBFadeValue_B = 255;
  RGBFadeDir_R = false;
  RGBFadeDir_G = false;
  RGBFadeDir_B = false;
  }
 if (function == RGBCYAN)
  {
  RGBValue_R = 0;
  RGBValue_G = 255;
  RGBValue_B = 255;
  RGBFadeValue_R = 0;
  RGBFadeValue_G = 255;
  RGBFadeValue_B = 255;
  RGBFadeDir_R = true;
  RGBFadeDir_G = false;
  RGBFadeDir_B = false; 
  }
 if (function == RGBYELLOW)
  {
  RGBValue_R = 255;
  RGBValue_G = 255;
  RGBValue_B = 0; 
  RGBFadeValue_R = 0;
  RGBFadeValue_G = 0;
  RGBFadeValue_B = 0;
  RGBFadeDir_R = true;
  RGBFadeDir_G = true;
  RGBFadeDir_B = true;
  }
  if (function == RGBMAGENTA)
  {
  RGBValue_R = 255;
  RGBValue_G = 0;
  RGBValue_B = 255;
  RGBFadeValue_R = 255;
  RGBFadeValue_G = 0;
  RGBFadeValue_B = 255;
  RGBFadeDir_R = false;
  RGBFadeDir_G = true;
  RGBFadeDir_B = false;
  }
 if (function == RGBSHORTBLACK)
  {
  analogWrite(RGBLED_R, 0); 
  analogWrite(RGBLED_G, 0);
  analogWrite(RGBLED_B, 0);  
  }
RGBFadeEnabled = fadeit;
if (!(RGBFadeEnabled))
  {
  analogWrite(RGBLED_R, RGBValue_R);
  analogWrite(RGBLED_G, RGBValue_G);
  analogWrite(RGBLED_B, RGBValue_B);  
  }   
}


void SerialHandler ()
{

if (mySerial.available()) 
  {
  inByte = mySerial.read();
  if (inByte == 30) // Eingabe gesperrt Zeitschloss aktiv
    {
    InputBlocked = true; 
    RGBControl(RGBRED,true); 
    }
  if (inByte == 40) // Eingabe entsperrt Zeitschloss deaktiviert
    {
    RGBControl(RGBMAGENTA,true);      
    InputBlocked = false;
    tone(PIEZOSUMMER, 880, 100);
    delay(120);   
    } 
  if (inByte == 20) // Code Correct
    {
    RGBControl(RGBGREEN,false);
    tone(PIEZOSUMMER, 1200, 200);
    delay(2000);
    PinEnteredFalseBefore  = false;
    RGBControl(RGBBLUE,true); // NORMAL MODE 
    } else 
    if (inByte == 21) // Code falsch
      {
      analogWrite(RGBLED_R, 255); 
      analogWrite(RGBLED_G, 0);
      analogWrite(RGBLED_B, 0); 
      tone(PIEZOSUMMER, 400, 300);
      delay(500);
      RGBControl(RGBRED,true);
      InputBlocked = true;
      PinEnteredFalseBefore  = true;
      }      
  if (inByte == 25) // Out of Sync
    {
    RGBControl(RGBYELLOW,true);
    InSync = false;
    InititalKey = 0; // Delete Encryption Key
    }
  if (inByte == 23) //Clear ausgeführt 
    {
    inByte = 0;
    }
  if (inByte == 22) // EIngabe azeptiert 
    {
    inByte = 0;
    }      
  }
}


void TimeMgmnt ()
{
if ((millis() - previousMillisKeyBoard > KeybModeTimeInterval1) & CodeEnterSeqence & InSync) // Auto Reset KEyboard Input
  {
  previousMillisKeyBoard = millis(); 
  tone(PIEZOSUMMER, 988, 100);
  delay(110);
  if (PinEnteredFalseBefore)
    {
    RGBControl(RGBMAGENTA,true); // NORMAL MODE - Pin entered false before
    } else 
    {
    RGBControl(RGBBLUE,true); // NORMAL MODE   
    }  
  CodeEnterSeqence = false;
  previousMillisKeyBoard = millis(); 
  byte randNumber = random(0, 254);
  EnCodedKeyStroke = 10 ^ randNumber;
  mySerial.write(EnCodedKeyStroke);
  }
if (millis() - previousMillis >  RGBFadeInterval1) //Fadint LEd's
  {
  if (RGBFadeEnabled)
    {  
    previousMillis = millis();   // aktuelle Zeit abspeichern
    if (RGBValue_B > 0)
      {
      if (RGBFadeDir_B)
        {
        RGBFadeValue_B++;
        if ( RGBFadeValue_B >=  RGBValue_B) {RGBFadeDir_B = false; }
        } else
        {
        RGBFadeValue_B--;
        if ( RGBFadeValue_B < 1) {RGBFadeDir_B = true; }
        }
      } else { RGBFadeValue_B = 0; }
    if (RGBValue_R > 0)
      {
        if (RGBFadeDir_R)
        {
        RGBFadeValue_R++;
        if ( RGBFadeValue_R >=  RGBValue_R) {RGBFadeDir_R = false; }
        } else
        {
        RGBFadeValue_R--;
        if ( RGBFadeValue_R < 1) {RGBFadeDir_R = true; }
        }
      } else { RGBFadeValue_R = 0; }
    if (RGBValue_G > 0)
      {
      if (RGBFadeDir_G)
      {
      RGBFadeValue_G++;
      if ( RGBFadeValue_G >=  RGBValue_G) {RGBFadeDir_G = false; }
      } else
      {
      RGBFadeValue_G--;
      if ( RGBFadeValue_G < 1) {RGBFadeDir_G = true; }
      }
      } else { RGBFadeValue_G = 0; }     
    analogWrite(RGBLED_R, RGBFadeValue_R);
    analogWrite(RGBLED_G, RGBFadeValue_G);
    analogWrite(RGBLED_B, RGBFadeValue_B);
    } 
  }
}

void KeyboardHandler(bool NotEnabled)
{
key = keypad.getKey();  
if((key)) // Key Entered
  {
  if (!NotEnabled)
    {  
    byte randNumber = random(0, 254);
    EnCodedKeyStroke = key ^ randNumber;
    mySerial.write(EnCodedKeyStroke);
    if((key == 10) | (key == 12)) 
      { 
        RGBControl(RGBSHORTBLACK,true);       
        tone(PIEZOSUMMER, 988, 100);
        delay(120);
        CodeEnterSeqence = false;
        if(key == 10)
          {  
          if (PinEnteredFalseBefore)
            {
            RGBControl(RGBMAGENTA,true); // NORMAL MODE - Pin entered false before
            } else 
            {
            RGBControl(RGBBLUE,true); // NORMAL MODE   
            }   
          }                                                                                                                            
      } else
      {
        RGBControl(RGBSHORTBLACK,true);
        tone(PIEZOSUMMER, 880, 100);
        delay(120);
        CodeEnterSeqence = true;
        RGBControl(RGBCYAN,true); 
        previousMillisKeyBoard = millis();   
      }
    } 
  }  
}


void loop()
{  
if (InSync)
  {
  KeyboardHandler(InputBlocked);
  }
TimeMgmnt ();
SerialHandler ();          
}
  
