#include <SoftwareSerial.h> 

#define RELAIS_A A0
#define Interval1 1000
#define MAXDelayStages 7

const byte MaxPinCodeLength = 20;
SoftwareSerial mySerial(2, 3); // RX, TX

byte KeyPadBuffer[MaxPinCodeLength];
byte PinCode[MaxPinCodeLength] = {1,2,3,13}; // Standard Pincode: 123A  - Bitte Ändern gemäß Beschreibung - 
byte BufferCount = 0;
byte a;
bool AcceptCode =false; 
long previousMillis = 0;

void setup()
{  
 Serial.begin(9600); 
 mySerial.begin(9600);
 pinMode(RELAIS_A,OUTPUT); //Relais Output
 digitalWrite(RELAIS_A,HIGH); //LOW Aktiv
} 


void loop() 
{
if (mySerial.available())
  { 
   byte DeCodedKeyStroke = mySerial.read(); 
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
        Serial.println("Auswertung gestartet");   
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
          AcceptCode = false;
          } else
          {
          mySerial.write(21);
          delay(2000);
          mySerial.write(40);  // Delay Mode End      
          }
        for (a = 0; a <= MaxPinCodeLength -1; a++) { KeyPadBuffer[a] = 0; } 
        Serial.println("Clear all Memory");
        BufferCount = 0;  
      } else
      {
       KeyPadBuffer[BufferCount] = DeCodedKeyStroke;       
       if (BufferCount < MaxPinCodeLength ) { BufferCount++; } 
       mySerial.write(22); 
      }      
  }
}
