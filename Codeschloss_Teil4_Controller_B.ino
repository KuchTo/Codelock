#include <SoftwareSerial.h> 

#define RELAIS_A A0
#define RELAIS_B A1
#define Interval1 1000
#define MAXDelayStages 7

const byte MaxPinCodeLength = 20;
const byte DelayInterationsInSec[MAXDelayStages] = {1,5,10,20,30,45,60}; 

//const byte DelayInterationsInSec[MAXDelayStages] = {1,1,1,1,1,1,1}; 

SoftwareSerial mySerial(2, 3); // RX, TX


byte KeyPadBuffer[MaxPinCodeLength];
byte PinCode[MaxPinCodeLength] = {1,2,3,13}; // Standard Pincode: 123A  - Bitte Ändern gemäß Beschreibung -  
byte BufferCount = 0;
byte a;
bool InSync = true;
bool AcceptCode =false; 
byte ErrorCounter = 0; 
long previousMillis = 0;
byte InputDelay = 0;
byte RecInititalKeyLength = 0;

unsigned long CommuncationKey = 902841;

union foo {
  byte as_array[4];
  long as_long;
} convert;

void setup()
{  
 Serial.begin(9600); 
 mySerial.begin(9600);
 pinMode(RELAIS_A,OUTPUT); //Relais Output
 digitalWrite(RELAIS_A,HIGH); //LOW Aktiv
 BufferCount = 0;
 for (a = 0; a <= MaxPinCodeLength -1 ; a++)
  {
  KeyPadBuffer[a] = 0;
  }
 convert.as_long = CommuncationKey;
 RecInititalKeyLength = 0;
 delay(5000);
 do
  {
  mySerial.write(convert.as_array[RecInititalKeyLength]);   //little Endian
  RecInititalKeyLength++;   
  } while (RecInititalKeyLength < 4);
 randomSeed(CommuncationKey);
} 


void loop() 
{
if (mySerial.available())
  { 
   byte randNumber = random(0, 254);
   byte key = mySerial.read();
   byte DeCodedKeyStroke = key ^ randNumber;  
   if ((DeCodedKeyStroke < 18) & InSync)
    {
     if(DeCodedKeyStroke == 10)   // Clear Keypad Buffer  Key: *
      { 
       for (a = 0; a <= MaxPinCodeLength -1; a++)
        {
        KeyPadBuffer[a] = 0;
        }      
       Serial.print("Clear ");
       Serial.println(BufferCount);
       mySerial.write(23);
       BufferCount = 0;
      } else     
     if(DeCodedKeyStroke ==12)   // Enter Keypad Buffer  Key: #
      {
      if (InputDelay == 0)
        {
        Serial.println("Auswertung gestertet");   
        Serial.println(BufferCount);
        AcceptCode = true; 
        for (a = 0; a <= MaxPinCodeLength -1 ; a++)
          {
          if (!(PinCode[a] == KeyPadBuffer[a])) {AcceptCode = false; } 
          Serial.print(PinCode[a]);
          Serial.print(";");
          Serial.print(KeyPadBuffer[a]);  
          Serial.println(" ");
          }
        Serial.println("END");  
        if (AcceptCode)
          {
          mySerial.write(20);
          digitalWrite(RELAIS_A,(!digitalRead(RELAIS_A)));
          ErrorCounter = 0;
          InputDelay = 0;
          AcceptCode = false;
          } else
          {
          mySerial.write(21);
          if ( ErrorCounter < MAXDelayStages - 1) { ErrorCounter++; }
          InputDelay = DelayInterationsInSec [ErrorCounter];
          }
        for (a = 0; a <= MaxPinCodeLength -1; a++) { KeyPadBuffer[a] = 0; } 
        Serial.println("Clear all Memory");
        BufferCount = 0;
        } else
        {
         Serial.println("Delay Mode Active");  
         mySerial.write(30);  // Delay Mode
         for (a = 0; a <= MaxPinCodeLength -1 ; a++) { KeyPadBuffer[a] = 0; }
         BufferCount = 0;
        }
      
      } else
      {
       KeyPadBuffer[BufferCount] = DeCodedKeyStroke;       
       if (BufferCount < MaxPinCodeLength ) { BufferCount++; } 
       if (InputDelay == 0) { mySerial.write(22); } else  { mySerial.write(30); } 
      }
      } else
      {
        //Out of Sync
        Serial.print("Out of sync Data: ");
        Serial.println(DeCodedKeyStroke);
        mySerial.write(25);
        if ( ErrorCounter < MAXDelayStages - 1) { ErrorCounter++; }
        InSync = false;
      }
  }

if (millis() - previousMillis > Interval1)
{
  // Auto Reset KEyboard Input
 ; 
   previousMillis = millis(); 
   if (InputDelay > 0)
    {
      if (InputDelay == 1)
      {
      Serial.println ("release");
      mySerial.write(40);  // Delay Mode End
      }
     InputDelay = InputDelay - 1;
    }
  }
}
