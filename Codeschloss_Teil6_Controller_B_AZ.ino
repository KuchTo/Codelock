// Codeschloss by Tobias Kuch 2020 GPL 3.0
// Repository: https://github.com/kuchto
// tobias.kuch@googlemail.com

#include <SoftwareSerial.h> 
#include <UIPEthernet.h>
#include <EEPROM.h>

#define RELAIS_A A0
#define RELAIS_B A1
#define MACADDRESS 0x00,0x01,0x02,0x03,0x04,0x05
#define MYIPADDR 192,168,1,6
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1
#define LISTENPORT 23
#define UARTBAUD 9600

#define Interval1 1000
#define MAXDelayStages 7

const byte MaxPinCodeLength = 20;
const byte DelayInterationsInSec[MAXDelayStages] = {1,5,10,20,30,45,60}; 

SoftwareSerial mySerial(5, 3); // RX, TX
EthernetServer server = EthernetServer(LISTENPORT);
EthernetClient client;
EthernetClient newClient;

byte KeyPadBuffer[MaxPinCodeLength];
byte BufferCount = 0;
byte a;
bool InSync = true;
byte ErrorCounter = 0; 
long previousMillis = 0;
byte InputDelay = 0;
byte RecInititalKeyLength = 0;
uint8_t mac[6] = {MACADDRESS};
unsigned long CommuncationKey = 902841;

union foo {
  byte as_array[4];
  long as_long;
} convert;

struct EEPromData
  {
   byte PinCodeA[MaxPinCodeLength];
   bool PinCodeAEnabled = false;
   byte PinCodeB[MaxPinCodeLength];
   bool PinCodeBEnabled = false;
   char ConfigValid[3]; //If Config is Vaild, Tag "TK" is required"
  };

EEPromData MyPinCodes;


void SaveCodesToEEPROM ()
{
  for (int i = 0 ; i < sizeof(MyPinCodes) ; i++) 
     {
      EEPROM.write(i, 0);
     }
  strncpy(MyPinCodes.ConfigValid , "TK", sizeof(MyPinCodes.ConfigValid) );
  EEPROM.put(0, MyPinCodes);
}

bool GetCodesFromEEPROM ()
{
 bool RetValue;
 EEPROM.get(0, MyPinCodes);
 if (String(MyPinCodes.ConfigValid) == String("TK")) 
  {
    RetValue = true;
  } else
  {
    RetValue = false; 
  }
  return RetValue;
}

void setup()
{  
 Serial.begin(9600); 
 mySerial.begin(9600);
 pinMode(RELAIS_A,OUTPUT); //Relais Output
 pinMode(RELAIS_B,OUTPUT); //Relais Output
 digitalWrite(RELAIS_A,HIGH); //LOW Aktiv
 digitalWrite(RELAIS_B,HIGH); //LOW Aktiv
 if (!(GetCodesFromEEPROM()))
  {
    Serial.println (F("Empty EEPROM."));
    MyPinCodes.PinCodeA[0] = 49; // Default Pincode: 123 
    MyPinCodes.PinCodeA[1] = 50;
    MyPinCodes.PinCodeA[2] = 51;
    MyPinCodes.PinCodeA[3] = 0;
    MyPinCodes.PinCodeAEnabled = true;
    MyPinCodes.PinCodeAEnabled = false;
    SaveCodesToEEPROM();
  }
 BufferCount = 0;
 for (a = 0; a <= MaxPinCodeLength -1 ; a++)
  {
  KeyPadBuffer[a] = 0;
  }
 convert.as_long = CommuncationKey;
 RecInititalKeyLength = 0;
 Ethernet.begin(mac);
 server.begin();
 Serial.println(Ethernet.localIP());  
 do
  {
  mySerial.write(convert.as_array[RecInititalKeyLength]);   //little Endian
  RecInititalKeyLength++;   
  } while (RecInititalKeyLength < 4);
 randomSeed(CommuncationKey);
} 


bool CheckEnabled (String s)
{
if (s == "ENABLED")
  {
  return true;
  } else
  {
  return false;  
  } 
}

bool CheckDisabled (String s)
{
if (s == "DISABLED")
  {
  return true;
  } else
  {
  return false;  
  } 
}


void loop() 
{
 if (client = server.available())
  {
  byte Position = 0;
  bool CommandReceived = false;
  char EtherNetCommand[MaxPinCodeLength + 7] = "";
  while((client.available()) > 0)
    {
    byte thisChar = client.read();
    if ((thisChar < 123) & (thisChar > 47) & (Position < MaxPinCodeLength + 6) ) // Sonderzeichen ausfiltern
      {
      EtherNetCommand[Position] = thisChar;
      Position++;
      if (Position > 1) { CommandReceived = true; }
      }
    }
     if (CommandReceived)
       {
       String s(EtherNetCommand);
       s.toUpperCase();
       EtherNetCommand[MaxPinCodeLength + 7] = "";
       if (s.startsWith("TOGGLE:"))
        {
          s.remove(0, 7);     
          if (s == "A")
            {
            digitalWrite(RELAIS_A,(!digitalRead(RELAIS_A))); 
            client.println(F("OK"));
            mySerial.write(20);
            } else
          if (s == "B")
            {
            digitalWrite(RELAIS_B,(!digitalRead(RELAIS_B))); 
            client.println(F("OK"));
            }
   
        } else     
      if (s.startsWith("LOCK:"))
      {
        s.remove(0, 5);   
        if (CheckEnabled(s))
          {
          client.println(F("OK"));
          mySerial.write(41);
          } else
        if (CheckDisabled(s))  
          {
          client.println(F("OK"));
          mySerial.write(21);
          } else { client.println(F("ERROR")); }  
      } else   
      if (s.startsWith("LED:"))
      {
        s.remove(0, 4);
        if (CheckEnabled(s))
          {
          mySerial.write(27);
          client.println(F("OK"));
          } else
        if (CheckDisabled(s))
          { 
          mySerial.write(26);
          client.println(F("OK"));
          } else { client.println(F("ERROR")); }
      } else
      if (s.startsWith("CODEA:"))
        {
          s.remove(0, 6);
          if (CheckEnabled(s))  //CheckEnabled // old: if (s.startsWith("ENABLED"))
            {
             MyPinCodes.PinCodeAEnabled = true;
             client.println(F("OK"));  
             SaveCodesToEEPROM ();
            } else
          if (CheckDisabled(s)) // if (s.startsWith("DISABLED")) 
            {
             MyPinCodes.PinCodeAEnabled = false;
             client.println(F("OK"));  
             SaveCodesToEEPROM ();
          } else
          {
            Serial.println(F("Neuer Code A"));
            byte a = s.length();
            bool CodeOk = true;
            for (Position = 0;Position < a;Position++) // Check auf gültige Zeichen
              {
              if ((char(s[Position])<48) | (char(s[Position])>68)| ((char(s[Position])>57) & (char(s[Position])<65))) { CodeOk = false; }            
              }
            if (CodeOk)
              {
              client.println(F("OK"));    
              Serial.println(s);
              for (Position = 0;Position < MaxPinCodeLength;Position++) // Check auf gültige Zeichen
                {
                  MyPinCodes.PinCodeA[Position] = 0;  
                }             
              for (Position = 0;Position < a;Position++) // Check auf gültige Zeichen
                {
                  MyPinCodes.PinCodeA[Position] = s[Position];  
                }           
              SaveCodesToEEPROM ();
              } else { client.println(F("ERROR")); }
          }
        } else       
      if (s.startsWith("CODEB:"))
        {
          s.remove(0, 6);
          if (CheckEnabled(s))
            {
             MyPinCodes.PinCodeBEnabled = true;
             client.println(F("OK"));  
             SaveCodesToEEPROM ();
            } else
          if (CheckDisabled(s))
            {
             MyPinCodes.PinCodeBEnabled = false;
             client.println(F("OK"));  
             SaveCodesToEEPROM ();
          } else
          {
            Serial.println(F("Neuer Code B"));
            byte a = s.length();
            bool CodeOk = true;
            for (Position = 0;Position < a;Position++) // Check auf gültige Zeichen
              {
              if ((char(s[Position])<48) | (char(s[Position])>68)| ((char(s[Position])>57) & (char(s[Position])<65))) { CodeOk = false; }            
              }
            if (CodeOk)
              {
              client.println(F("OK"));    
              Serial.println(s);
              for (Position = 0;Position < MaxPinCodeLength;Position++) // Check auf gültige Zeichen
                {
                  MyPinCodes.PinCodeB[Position] = 0;  
                }             
              for (Position = 0;Position < a;Position++) // Check auf gültige Zeichen
                {
                  MyPinCodes.PinCodeB[Position] = s[Position];  
                }           
              SaveCodesToEEPROM();
              } else { client.println(F("ERROR")); }
          }
        } else { client.println(F("ERROR")); }
      }
   }  
 if (client && !client.connected()) 
  {
  Serial.print(F("Client Disconnected"));
  client.stop();
  }
    
if (mySerial.available())
  { 
   byte randNumber = random(0, 254);
   byte key = mySerial.read();
   byte DeCodedKeyStroke = key ^ randNumber;  
   if (((DeCodedKeyStroke > 47) & (DeCodedKeyStroke < 69))  & InSync)
    {
     if(DeCodedKeyStroke == 58)   // Clear Keypad Buffer  Key: *
      { 
       for (a = 0; a <= MaxPinCodeLength -1; a++)
        {
        KeyPadBuffer[a] = 0;
        }      
       Serial.print(F("Clear "));
    //   Serial.println(BufferCount);
       mySerial.write(23);
       BufferCount = 0;
      } else         
     if(DeCodedKeyStroke ==59)   // Enter Keypad Buffer  Key: #
      {
      if (InputDelay == 0)
        {
        //Serial.println("Auswertung gestartet");   // Zu Debugzwecken
        // Serial.println(BufferCount);
        bool AcceptCodeA = true;
        bool AcceptCodeB = true;  
        if (MyPinCodes.PinCodeAEnabled)
          {
          for (a = 0; a <= MaxPinCodeLength -1 ; a++)
            {
            if (!(MyPinCodes.PinCodeA[a] == KeyPadBuffer[a])) {AcceptCodeA = false; } 
            //Serial.print(MyPinCodes.PinCodeA[a]); // Zu Debugzwecken
            //Serial.print(";");
            //Serial.print(KeyPadBuffer[a]);  
            // Serial.println(" ");
            }
          } else {AcceptCodeA = false; } 
        if (MyPinCodes.PinCodeBEnabled)
          {
          for (a = 0; a <= MaxPinCodeLength -1 ; a++)
            {
            if (!(MyPinCodes.PinCodeB[a] == KeyPadBuffer[a])) {AcceptCodeB = false; } 
            }
          } else {AcceptCodeB = false; }         
        // Serial.println("END");  // Zu Debugzwecken    
        if (AcceptCodeA | AcceptCodeB)
          {
          mySerial.write(20);
          digitalWrite(RELAIS_A,(!digitalRead(RELAIS_A)));
          ErrorCounter = 0;
          InputDelay = 0;
          AcceptCodeA = false;
          AcceptCodeB = false;
          } else
          {
          mySerial.write(21);
          if ( ErrorCounter < MAXDelayStages - 1) { ErrorCounter++; }
          InputDelay = DelayInterationsInSec [ErrorCounter];
          }
        for (a = 0; a <= MaxPinCodeLength -1; a++) { KeyPadBuffer[a] = 0; } 
        Serial.println(F("Clearing Memory"));
        BufferCount = 0;
        } else
        {
         Serial.println(F("Delay Mode Active"));  
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
        Serial.print(F("Out of sync Data: "));
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
      Serial.println (F("Release"));
      mySerial.write(40);  // Delay Mode End
      }
     InputDelay = InputDelay - 1;
    }
  }
}
