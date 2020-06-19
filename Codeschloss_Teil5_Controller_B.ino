// Codeschloss Tobias Kuch 2020 GPL 3.0
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
//byte PinCode[MaxPinCodeLength] = {1,2,3,13}; // Standard Pincode: 123A  - Bitte Ändern gemäß Beschreibung - 
byte BufferCount = 0;
byte a;
bool InSync = true;
bool AcceptCode =false;
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
 digitalWrite(RELAIS_A,HIGH); //LOW Aktiv
 if (GetCodesFromEEPROM())
  {
    Serial.println (F("Codes from EEPROM loaded."));
  } else
  {
    Serial.println (F("Empty EEPROM found. Set default Code."));
    MyPinCodes.PinCodeA[0] = 49; // Default Pincode: 123 
    MyPinCodes.PinCodeA[1] = 50;
    MyPinCodes.PinCodeA[2] = 51;
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
     //  EtherNetCommand[MaxPinCodeLength + 7] = "";
      if (s.startsWith("CODEA:"))
        {
          s.remove(0, 6);
          Serial.println(F("Neuer Code"));
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
       Serial.println(BufferCount);
       mySerial.write(23);
       BufferCount = 0;
      } else     
     if(DeCodedKeyStroke ==59)   // Enter Keypad Buffer  Key: #
      {
      if (InputDelay == 0)
        {
        //Serial.println("Auswertung gestartet");   // Zu Debugzwecken
        // Serial.println(BufferCount);
        AcceptCode = true; 
        for (a = 0; a <= MaxPinCodeLength -1 ; a++)
          {
          if (!(MyPinCodes.PinCodeA[a] == KeyPadBuffer[a])) {AcceptCode = false; } 
          //Serial.print(MyPinCodes.PinCodeA[a]); // Zu Debugzwecken
          //Serial.print(";");
          //Serial.print(KeyPadBuffer[a]);  
          // Serial.println(" ");
          }
        // Serial.println("END");  // Zu Debugzwecken
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
