// Codeschloss Tobias Kuch 2020 GPL 3.0
#include <Keypad.h> 

#define RGBLED_G 2
#define RGBLED_B 4 
#define RGBLED_R 3 
#define RELAIS_A A0

const byte ROWS = 4; 
const byte COLS = 4; 
const byte MaxPinCodeLength = 20;
 
char keys[ROWS][COLS] = { 
                          {1,2,3,13},  
                          {4,5,6,23}, 
                          {7,8,9,33},  
                          {40,10,42,43}
                         }; 
byte rowPins[ROWS] = {8,7,6,5}; //5,6,7,8};
byte colPins[COLS] = {12,11,10,9}; // {9,10,11,12};
byte KeyPadBuffer[MaxPinCodeLength];
byte PinCode[MaxPinCodeLength] = {1,2,3,13}; // Standard Pincode: 123A  - Bitte Ändern gemäß Beschreibung - 
byte BufferCount = 0;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

void setup()
{  
 Serial.begin(9600); 
 pinMode(RGBLED_G,OUTPUT); // Ausgang RGB LED Grün
 pinMode(RGBLED_R,OUTPUT); // Ausgang RGB LED Rot
 pinMode(RGBLED_B,OUTPUT); // Ausgang RGB LED Blau
 pinMode(RELAIS_A,OUTPUT); //Relais Output
 digitalWrite(RELAIS_A,HIGH);
 BufferCount = 0;
 for (byte a = 0; a <= MaxPinCodeLength; a++)
  {
  KeyPadBuffer[a] = 0;
  }
 digitalWrite(RGBLED_R,LOW);
 digitalWrite(RGBLED_G,HIGH);
 digitalWrite(RGBLED_B,HIGH);  
} 

void loop()
{  
   char key = keypad.getKey();  
   if(key)
   {
     if(key ==40)   // Clear Keypad Buffer  Key: *
      { 
       for (byte a = 0; a <= MaxPinCodeLength; a++)
        {
        KeyPadBuffer[a] = 0;
        }      
       digitalWrite(RGBLED_R,HIGH);
       digitalWrite(RGBLED_G,LOW);
       digitalWrite(RGBLED_B,HIGH); 
       Serial.print("Clear ");
       Serial.println(BufferCount);
       BufferCount = 0;
      } else     
     if(key ==42)   // Enter Keypad Buffer  Key: #
      {
       bool AcceptCode = true; 
       for (byte a = 0; a <= BufferCount; a++)
        {
        if (!(PinCode[a] == KeyPadBuffer[a])) {AcceptCode = false; } 
        }
        if (AcceptCode)
          {
            Serial.println("Code korrekt");   
            digitalWrite(RELAIS_A,(!digitalRead(RELAIS_A)));
            digitalWrite(RGBLED_R,LOW);
            digitalWrite(RGBLED_G,HIGH);
            digitalWrite(RGBLED_B,LOW);
          } else
          {
            digitalWrite(RGBLED_R,HIGH);
            digitalWrite(RGBLED_G,LOW);
            digitalWrite(RGBLED_B,LOW);
          }
        for (byte a = 0; a <= MaxPinCodeLength; a++) { KeyPadBuffer[a] = 0; } 
        BufferCount = 0;
        delay(1000);
        digitalWrite(RGBLED_R,LOW);
        digitalWrite(RGBLED_G,HIGH);
        digitalWrite(RGBLED_B,HIGH);  
      } else
      {
       KeyPadBuffer[BufferCount] = key;       
       if (BufferCount < MaxPinCodeLength ) { BufferCount++; }    
      }
   } 
}
  
