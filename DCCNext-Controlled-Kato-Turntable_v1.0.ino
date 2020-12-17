//----------------------------------------------------------// DCCNext-Controlled-Kato-Turntable_v1.34
#include <EEPROM.h>                                         // Standard Arduino EEPROM library
#include <DCC_Decoder.h>                                    // Use Manage Libraries to add: NmraDcc -- https://github.com/MynaBay/DCC_Decoder
#include <ezButton.h>                                       // Use Manage Libraries to add: ezButton -- https://github.com/ArduinoGetStarted/button
#include <LiquidCrystal_I2C.h>                              // Use Manage Libraries to add: LiquidCrystal I2C -- https://github.com/johnrickman/LiquidCrystal_I2C
#define maxSpeed                       100                  // Speed between -255 = Reversed to 255 = Forward (-5 to +5 VDC)
#define maxTrack                        36                  // Total Number of Turntable Tracks
#define kDCC_INTERRUPT                   0                  // DCC Interrupt 0
#define DCC_Max_Accessories             22                  // Total Number of DCC Accessory Decoder Addresses = 225-246
#define DCC_Interrupt                    2                  // Arduino Output Pin  2 = DCC signal = Interrupt 0
// #define Turntable_Switch                 3                  // Arduino Output Pin  3 = Turntable Trigger      = Cable Pin 1
#define Turntable_MotorM1                5                  // Arduino Output Pin  5 = Turntable Motor M1     = Cable Pin 3
#define Turntable_MotorM2                6                  // Arduino Output Pin  6 = Turntable Motor M1     = Cable Pin 4
#define Turntable_BridgeL                7                  // Arduino Output Pin  7 = Turntable Bridge Left  = Cable Pin 7
#define Turntable_BridgeR                8                  // Arduino Output Pin  8 = Turntable Bridge Right = Cable Pin 8
#define Turntable_LockL2                 9                  // Arduino Output Pin  9 = Turntable Lock L2      = Cable Pin 6
#define Turntable_LockL1                10                  // Arduino Output Pin 10 = Turntable Lock L1      = Cable Pin 5
#define Turntable_Status                13                  // Arduino Output LED 13 = Bridge in Position
                                                            // Arduino Output Pin 14 = Red LED           = Function Red
                                                            // Arduino Output Pin 15 = Green LED         = Function Green
                                                            // Arduino Output Pin 16 = Yellow LED        = TURN 180
                                                            // Arduino Output Pin 17 = Blue LED          = ????????
ezButton Turntable_Switch(3);                               // Arduino Output Pin  3 = Turntable Trigger      = Cable Pin 1
ezButton Button_180(4);                                     // Arduino Input  Pin  4 = Button Turn 180   = Turn 180
ezButton Button_Right(11);                                  // Arduino Input  Pin 11 = Button Turn Right = Turn 1 Step ClockWise
ezButton Button_Left(12);                                   // Arduino Input  Pin 12 = Button Turn Left  = Turn 1 Step Counter ClockWise
uint8_t DCC_Action_LED              =    0;                 // Pin Number will change by DCC command
uint8_t Turntable_CurrentTrack      =    0;                 // Turntable Current Track
uint8_t Turntable_NewTrack          =    0;                 // Turntable New Track
uint8_t EE_Address                  =    0;                 // EEPROM Address Turntable Bridge Position
int speedValue                      =    0;                 // Turntable Motor Speed = 0 - 255
boolean Turntable_NewSwitchState    =  LOW;                 // New Switch Status (From HIGH to LOW = Turntable Bridge in Position)
boolean Turntable_OldSwitchState    =  LOW;                 // Old Switch Status (HIGH = Turntable bridge not in position)
unsigned long Turntable_TurnStart   =    0;                 // Start time to turn before stop
unsigned long Turntable_TurnTime    = 1000;                 // Minimum time in ms to turn before check to stop
unsigned long Turntable_SwitchTime  =    0;                 // Last time the output pin was toggled
unsigned long Turntable_SwitchDelay =  100;                 // Debounce time in ms


const char* Turntable_States[] =                            // Possible Turntable States
{//012345678
  "TCW      ",                                              // Turn ClockWise
  "TCCW     ",                                              // Turn Counter ClockWise
  "MCW      ",                                              // Motor ClockWise
  "MCCW     ",                                              // Motor Counter ClockWise
  "STOP     ",                                              // Stop Turning
  "POS      ",                                              // Bridge in Position
  "DCC_END  ",                                              // DCC Command END
  "DCC_INPUT",                                              // DCC Command INPUT
  "DCC_CLEAR",                                              // DCC Command CLEAR
  "DCC_T180 ",                                              // DCC Command Turn 180
  "DCC_T1CW ",                                              // DCC Command Turn 1 Step ClockWise
  "DCC_T1CCW",                                              // DCC Command Turn 1 Step Counter ClockWise
  "BUT_T180 ",                                              // Button T180  = Turn 180
  "BUT_T1CW ",                                              // Button Right = Turn 1 Step ClockWise
  "BUT_T1CCW"                                               // Button Left  = Turn 1 Step Counter ClockWise
}; // END const

enum Turntable_NewActions:uint8_t                           // Possible Turntable Actions
{
  TCW         ,                                             // Turn ClockWise
  TCCW        ,                                             // Turn Counter ClockWise
  MCW         ,                                             // Motor ClockWise
  MCCW        ,                                             // Motor Counter ClockWise
  STOP        ,                                             // Stop Turning
  POS         ,                                             // Bridge in Position
  DCC_END     ,                                             // DCC Command END
  DCC_INPUT   ,                                             // DCC Command INPUT
  DCC_CLEAR   ,                                             // DCC Command CLEAR
  DCC_T180    ,                                             // DCC Command Turn 180
  DCC_T1CW    ,                                             // DCC Command Turn 1 Step ClockWise
  DCC_T1CCW   ,                                             // DCC Command Turn 1 Step Counter ClockWise
  Button_T180 ,                                             // Button T180  = Turn 180
  Button_T1CW ,                                             // Button Right = Turn 1 Step ClockWise
  Button_T1CCW                                              // Button Left  = Turn 1 Step Counter ClockWise
}; // END enum

enum Turntable_NewActions Turntable_OldAction = STOP;       // Stores Turntable Previous Action
enum Turntable_NewActions Turntable_NewAction = STOP;       // Stores Turntable New Action
enum Turntable_NewActions Turntable_Action    = STOP;       // Stores Turntable Requested Action

typedef struct                                              // Begin DCC Accessory Structure
{
  int               Address;                                // DCC Address to respond to
  uint8_t           Button;                                 // Accessory Button: 0 = Off (Red), 1 = On (Green)
  uint8_t           Position0;                              // Turntable Position0
  uint8_t           Position1;                              // Turntable Position1
  uint8_t           OutputPin1;                             // Arduino Output Pin 1
  uint8_t           OutputPin2;                             // Arduino Output Pin 2
  boolean           ReverseTrack0;                          // Reverse Track Power: 0 = Normal, 1 = Reversed
  boolean           ReverseTrack1;                          // Reverse Track Power: 0 = Normal, 1 = Reversed
  boolean           Finished;                               // Command Busy = 0 or Finished = 1 (Ready for next command)
  boolean           Active;                                 // Command Not Active = 0, Active = 1
  unsigned long     durationMilli;                          // Pulse Time in ms
  unsigned long     offMilli;                               // For internal use  // Do not change this value
} // END typedef
DCC_Accessory_Structure;                                    // End DCC Accessory Structure

DCC_Accessory_Structure DCC_Accessory[DCC_Max_Accessories]; // Define DCC_Accessory as DCC Accessory Structure
LiquidCrystal_I2C lcd(0x27, 20, 4);                         // I2C Liquid Crystal Display on Address 0x27 with 20 characters by 4 rows


void setup()
{
  Serial.begin(57600);
  Serial.println(F("DCCNext-Controlled-Kato-Turntable_v1.34 -- (c)JMRRvS 2020-12-14")); // Serial print loaded sketch
  lcd.init();                                               // Initialise LCD
  lcd.backlight();                                          // Turn backlight On
  lcd.setCursor(0, 0);                                      // Set cursor to first line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("DCCNext Controlled  "));                     // LCD print text
  lcd.setCursor(0, 1);                                      // Set cursor to second line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("Kato Turntable v1.34"));                     // LCD print text
  lcd.setCursor(0, 2);                                      // Set cursor to third line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("--------------------"));                     // LCD print text
  lcd.setCursor(0, 3);                                      // Set cursor to fourth line and left corner
//             01234567890123456789                         // Sample text
  Turntable_Switch.setDebounceTime(50);                     // set debounce time to 50 milliseconds
  Button_180.setDebounceTime(50);                           // set debounce time to 50 milliseconds
  Button_Right.setDebounceTime(50);                         // set debounce time to 50 milliseconds
  Button_Left.setDebounceTime(50);                          // set debounce time to 50 milliseconds
  pinMode(DCC_Interrupt    , INPUT_PULLUP);                 // Arduino Output Pin  2 = DCC signal = Interrupt 0
//  pinMode(Turntable_Switch , INPUT_PULLUP);                 // Arduino Output Pin  3 = Turntable Trigger      = Cable Pin 1
  pinMode(Turntable_MotorM1, OUTPUT);                       // Arduino Output Pin  5 = Turntable Motor M1     = Cable Pin 3
  pinMode(Turntable_MotorM2, OUTPUT);                       // Arduino Output Pin  6 = Turntable Motor M1     = Cable Pin 4
  pinMode(Turntable_BridgeL, OUTPUT);                       // Arduino Output Pin  7 = Turntable Bridge Left  = Cable Pin 7
  pinMode(Turntable_BridgeR, OUTPUT);                       // Arduino Output Pin  8 = Turntable Bridge Right = Cable Pin 8
  pinMode(Turntable_LockL2 , OUTPUT);                       // Arduino Output Pin  9 = Turntable Lock L2      = Cable Pin 6
  pinMode(Turntable_LockL1 , OUTPUT);                       // Arduino Output Pin 10 = Turntable Lock L1      = Cable Pin 5
  pinMode(Turntable_Status , OUTPUT);                       // Arduino Output LED 13 = Bridge in Position
  digitalWrite(Turntable_MotorM1, LOW);                     // Arduino Output Pin  5 = Turntable Motor M1     = Cable Pin 3
  digitalWrite(Turntable_MotorM2, LOW);                     // Arduino Output Pin  6 = Turntable Motor M1     = Cable Pin 4
  digitalWrite(Turntable_BridgeL, LOW);                     // Arduino Output Pin  7 = Turntable Bridge Left  = Cable Pin 7
  digitalWrite(Turntable_BridgeR, LOW);                     // Arduino Output Pin  8 = Turntable Bridge Right = Cable Pin 8
  digitalWrite(Turntable_LockL2 , LOW);                     // Arduino Output Pin  9 = Turntable Lock L2      = Cable Pin 6
  digitalWrite(Turntable_LockL1 , LOW);                     // Arduino Output Pin 10 = Turntable Lock L1      = Cable Pin 5
  digitalWrite(Turntable_Status , LOW);                     // Arduino Output LED 13 = Bridge in Position
                                                            // Arduino Output Pin 14 = Red LED           = Function Red
                                                            // Arduino Output Pin 15 = Green LED         = Function Green
                                                            // Arduino Output Pin 16 = Yellow LED        = TURN 180
                                                            // Arduino Output Pin 17 = Blue LED          = DCC_INPUT & DCC_CLEAR
  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC_Accessory_ConfigureDecoderFunctions();
  DCC.SetupDecoder( 0x00, 0x00, kDCC_INTERRUPT );
  for (int i = 0; i < DCC_Max_Accessories; i++)
  {
    DCC_Accessory[i].Button = 0;                            // Switch off all DCC decoders addresses
  } // END for
  Turntable_CurrentTrack = EEPROM.read(EE_Address);         // Read Turntable Bridge Position from EEPROM
  if ((Turntable_CurrentTrack < 1) || (Turntable_CurrentTrack > maxTrack))
  {
    Turntable_CurrentTrack = 0;                             // Reset CurrentTrack
    Turntable_NewTrack = 0;                                 // Reset NewTrack
  } // END if
  else
  {
    Turntable_NewTrack = Turntable_CurrentTrack;            // Start at Stored CurrentTrack
    digitalWrite(Turntable_Status, HIGH);                   // Arduino Output LED 13 = Bridge in Position
  } // END else

  delay(2000);
  for (int DCC_Action_LED = 14; DCC_Action_LED <= 17; DCC_Action_LED++)
  {
    pinMode(DCC_Action_LED, OUTPUT);                        // DCC Action LEDs as Output
    digitalWrite(DCC_Action_LED, HIGH);                     // Turn LED On at startup
    delay(200);                                             // LED On for 200 msec
    digitalWrite(DCC_Action_LED, LOW);                      // Turn LED Off at startup
  } // END for
} // END setup


void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data)
{
  address -= 1;
  address *= 4;
  address += 1;
  address += (data & 0x06) >> 1;                            // Convert NMRA packet address format to human address
  boolean enable = (data & 0x01) ? 1 : 0;
  for(int i = 0; i < DCC_Max_Accessories; i++)
  {
    if (address == DCC_Accessory[i].Address)
    {
      DCC_Accessory[i].Active = 1;                          // DCC Accessory Active
      if (enable)
      {
        DCC_Accessory[i].Button = 1;                        // Green Button
      } // END if
      else
      {
        DCC_Accessory[i].Button = 0;                        // Red Button
      } // END else
    } // END if
  } // END for
} // END BasicAccDecoderPacket_Handler


void DCC_Accessory_ConfigureDecoderFunctions()
{
  DCC_Accessory[0].Address        =   225;                  // DCC Address 225 0 = END, 1 = INPUT
  DCC_Accessory[0].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[0].Position0      =     0;                  // Turntable Position0 - not used in this function
  DCC_Accessory[0].Position1      =     0;                  // Turntable Position1 - not used in this function
//  DCC_Accessory[0].OutputPin1     =    xx;                  // Arduino Output Pin xx = LED xx - not used in this function
  DCC_Accessory[0].OutputPin2     =    17;                  // Arduino Output Pin 17 = Blue LED          = DCC_INPUT & DCC_CLEAR
  DCC_Accessory[0].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[0].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[0].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[0].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[0].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[1].Address        =   226;                  // DCC Address 226 0 = CLEAR, 1 = TURN 180
  DCC_Accessory[1].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[1].Position0      =     0;                  // Turntable Position0 - not used in this function
  DCC_Accessory[1].Position1      =     0;                  // Turntable Position1 - not used in this function
  DCC_Accessory[1].OutputPin1     =    17;                  // Arduino Output Pin 17 = Blue LED          = DCC_INPUT & DCC_CLEAR
  DCC_Accessory[1].OutputPin2     =    16;                  // Arduino Output Pin 16 = Yellow LED
  DCC_Accessory[1].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[1].ReverseTrack1  =     1;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[1].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[1].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[1].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[2].Address        =   227;                  // DCC Address 227 0 = 1 STEP CW, 1 = 1 STEP CCW
  DCC_Accessory[2].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[2].Position0      =     0;                  // Turntable Position0 - not used in this function
  DCC_Accessory[2].Position1      =     0;                  // Turntable Position1 - not used in this function
  DCC_Accessory[2].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[2].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[2].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[2].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[2].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[2].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[2].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[3].Address        =   228;                  // DCC Address 228 0 = Direction CW, 1 = Direction CCW
  DCC_Accessory[3].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[3].Position0      =     0;                  // Turntable Position0 - not used in this function
  DCC_Accessory[3].Position1      =     0;                  // Turntable Position0 - not used in this function
  DCC_Accessory[3].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[3].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[3].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[3].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[3].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[3].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[3].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[4].Address        =   229;                  // DCC Address 229 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[4].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[4].Position0      =     1;                  // Turntable Track 1
  DCC_Accessory[4].Position1      =     2;                  // Turntable Track 2
  DCC_Accessory[4].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[4].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[4].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[4].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[4].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[4].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[4].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[5].Address        =   230;                  // DCC Address 230 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[5].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[5].Position0      =     3;                  // Turntable Track 3
  DCC_Accessory[5].Position1      =     4;                  // Turntable Track 4
  DCC_Accessory[5].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[5].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[5].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[5].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[5].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[5].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[5].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[6].Address        =   231;                  // DCC Address 231 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[6].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[6].Position0      =     5;                  // Turntable Track 5
  DCC_Accessory[6].Position1      =     6;                  // Turntable Track 6
  DCC_Accessory[6].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[6].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[6].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[6].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[6].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[6].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[6].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[7].Address        =   232;                  // DCC Address 232 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[7].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[7].Position0      =     7;                  // Turntable Track 7
  DCC_Accessory[7].Position1      =     8;                  // Turntable Track 8
  DCC_Accessory[7].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[7].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[7].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[7].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[7].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[7].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[7].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[8].Address        =   233;                  // DCC Address 233 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[8].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[8].Position0      =     9;                  // Turntable Track 9
  DCC_Accessory[8].Position1      =    10;                  // Turntable Track 10
  DCC_Accessory[8].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[8].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[8].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[8].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[8].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[8].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[8].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[9].Address        =   234;                  // DCC Address 234 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[9].Button         =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[9].Position0      =    11;                  // Turntable Track 11
  DCC_Accessory[9].Position1      =    12;                  // Turntable Track 12
  DCC_Accessory[9].OutputPin1     =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[9].OutputPin2     =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[9].ReverseTrack0  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[9].ReverseTrack1  =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[9].Finished       =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[9].Active         =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[9].durationMilli  =   250;                  // Pulse Time in ms

  DCC_Accessory[10].Address       =   235;                  // DCC Address 235 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[10].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[10].Position0     =    13;                  // Turntable Track 13
  DCC_Accessory[10].Position1     =    14;                  // Turntable Track 14
  DCC_Accessory[10].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[10].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[10].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[10].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[10].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[10].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[10].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[11].Address       =   236;                  // DCC Address 236 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[11].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[11].Position0     =    15;                  // Turntable Track 15
  DCC_Accessory[11].Position1     =    16;                  // Turntable Track 16
  DCC_Accessory[11].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[11].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[11].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[11].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[11].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[11].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[11].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[12].Address       =   237;                  // DCC Address 237 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[12].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[12].Position0     =    17;                  // Turntable Track 17
  DCC_Accessory[12].Position1     =    18;                  // Turntable Track 18
  DCC_Accessory[12].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[12].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[12].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[12].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[12].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[12].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[12].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[13].Address       =   238;                  // DCC Address 238 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[13].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[13].Position0     =    19;                  // Turntable Track 19
  DCC_Accessory[13].Position1     =    20;                  // Turntable Track 20
  DCC_Accessory[13].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[13].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[13].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[13].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[13].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[13].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[13].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[14].Address       =   239;                  // DCC Address 239 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[14].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[14].Position0     =    21;                  // Turntable Track 21
  DCC_Accessory[14].Position1     =    22;                  // Turntable Track 22
  DCC_Accessory[14].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[14].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[14].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[14].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[14].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[14].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[14].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[15].Address       =   240;                  // DCC Address 240 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[15].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[15].Position0     =    23;                  // Turntable Track 23
  DCC_Accessory[15].Position1     =    24;                  // Turntable Track 24
  DCC_Accessory[15].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[15].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[15].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[15].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[15].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[15].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[15].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[16].Address       =   241;                  // DCC Address 241 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[16].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[16].Position0     =    25;                  // Turntable Track 25
  DCC_Accessory[16].Position1     =    26;                  // Turntable Track 26
  DCC_Accessory[16].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[16].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[16].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[16].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[16].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[16].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[16].durationMilli =   250;                  // Pulse Time in ms
  
  DCC_Accessory[17].Address       =   242;                  // DCC Address 242 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[17].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[17].Position0     =    27;                  // Turntable Track 27
  DCC_Accessory[17].Position1     =    28;                  // Turntable Track 28
  DCC_Accessory[17].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[17].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[17].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[17].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[17].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[17].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[17].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[18].Address       =   243;                  // DCC Address 243 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[18].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[18].Position0     =    29;                  // Turntable Track 29
  DCC_Accessory[18].Position1     =    30;                  // Turntable Track 30
  DCC_Accessory[18].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[18].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[18].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[18].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[18].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[18].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[18].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[19].Address       =   244;                  // DCC Address 244 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[19].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[19].Position0     =    31;                  // Turntable Track 31
  DCC_Accessory[19].Position1     =    32;                  // Turntable Track 32
  DCC_Accessory[19].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[19].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[19].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[19].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[19].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[19].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[19].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[20].Address       =   245;                  // DCC Address 245 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[20].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[20].Position0     =    33;                  // Turntable Track 33
  DCC_Accessory[20].Position1     =    34;                  // Turntable Track 34
  DCC_Accessory[20].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[20].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[20].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[20].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[20].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[20].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[20].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[21].Address       =   246;                  // DCC Address 246 0 = Goto Position0 , 1 = Position1
  DCC_Accessory[21].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[21].Position0     =    35;                  // Turntable Track 35
  DCC_Accessory[21].Position1     =    36;                  // Turntable Track 36
  DCC_Accessory[21].OutputPin1    =    14;                  // Arduino Output Pin 14 = Red LED
  DCC_Accessory[21].OutputPin2    =    15;                  // Arduino Output Pin 15 = Green LED
  DCC_Accessory[21].ReverseTrack0 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[21].ReverseTrack1 =     0;                  // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[21].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[21].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[21].durationMilli =   250;                  // Pulse Time in ms

  for (int i = 0; i < DCC_Max_Accessories; i++)             // Configure Arduino Output Pin
  {
    if (DCC_Accessory[i].OutputPin1)
    {
      pinMode(DCC_Accessory[i].OutputPin1, OUTPUT);
      digitalWrite(DCC_Accessory[i].OutputPin1, LOW);
    } // END if
    if (DCC_Accessory[i].OutputPin2)
    {
      pinMode(DCC_Accessory[i].OutputPin2, OUTPUT);
      digitalWrite(DCC_Accessory[i].OutputPin2, LOW);
    } // END if
  } // END for
} // END DCC_Accessory_ConfigureDecoderFunctions


void DCC_Accessory_CheckStatus()
{
  static int addr = 0;                                      // Begin loop through DCC Addresses
  DCC.loop();                                               // Loop DCC Library
  if (DCC_Accessory[addr].Finished && DCC_Accessory[addr].Active)
  {
    DCC_Accessory[addr].Finished = 0;
    DCC_Accessory[addr].offMilli = millis() + DCC_Accessory[addr].durationMilli;
    Serial.print(F("Address: "));
    Serial.print(DCC_Accessory[addr].Address);
    Serial.print(F(", "));
    Serial.print(F("Button: "));
    Serial.print(DCC_Accessory[addr].Button);
    Serial.print(F(" ("));
    Serial.print((DCC_Accessory[addr].Button) ? "Green" : "Red");  // 0 = Red, 1 = Green
    Serial.print(F(")"));
    Serial.println();
    switch (DCC_Accessory[addr].Address)
    {
      case (225):                                           // DCC Address 225 0 = END, 1 = INPUT
        if (DCC_Accessory[addr].Button == 0)                // Red Button    : 0 = END
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin1;  // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_CurrentTrack;      // Stop at Current Track
          Turntable_OldAction = STOP;                       // Action: Stop Motor M1
          Turntable_NewAction = DCC_END;                    // Action: Stop Motor M1
          Turntable_Action = DCC_END;                       // Requested Action = STOP
        } // END if

        if (DCC_Accessory[addr].Button == 1)                // Green Button  : 1 = INPUT
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin2;  // Set Arduino Output Pin
          Turntable_OldAction = STOP;                       // Action: Stop Motor M1
          Turntable_NewAction = DCC_INPUT;                  // Action: Stop Motor M1
          Turntable_Action = DCC_INPUT;                     // Requested Action = DCC_INPUT
        } // END if
        break;

      case (226):                                           // DCC Address 226 0 = CLEAR, 1 = TURN 180
        if (DCC_Accessory[addr].Button == 0)                // Red Button    : 0 = CLEAR
        {
          Turntable_CurrentTrack = 1;                       // Bridge in Home Position
          Turntable_NewTrack = 1;                           // Bridge in Home Position
          DCC_Action_LED = DCC_Accessory[addr].OutputPin1;  // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;        // Switch Old Action
          Turntable_NewAction = DCC_CLEAR;                  // Action: ???
          Turntable_Action = DCC_CLEAR;                     // Requested Action = DCC_CLEAR
        } // END if

        if (DCC_Accessory[addr].Button == 1)                // Green Button  : 1 = TURN 180
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin2;  // Set Arduino Output Pin
          if (Turntable_CurrentTrack < 19)
          {
            Turntable_NewTrack = Turntable_CurrentTrack + (maxTrack / 2);
          } // END if
          else
          {
            Turntable_NewTrack = Turntable_CurrentTrack - (maxTrack / 2);
          } // END else
          Turntable_OldAction = Turntable_NewAction;        // Switch Old Action
          Turntable_NewAction = DCC_T180;                   // Action: Turn Motor M1 (maxTrack / 2) Steps
          Turntable_Action = DCC_T180;                      // Requested Action = DCC_T180
        } // END if
        break;

      case (227):                                           // DCC Address 227 0 = 1 STEP CW, 1 = 1 STEP CCW
        if (DCC_Accessory[addr].Button == 0)                // Red Button   : 0 = TURN 1 Step ClockWise
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin1;  // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_CurrentTrack + 1;
          if (Turntable_NewTrack > maxTrack)                // From Track 36 to Track 1
          {
            Turntable_NewTrack = 1;                         // Track (1)
          } // END if
          Turntable_OldAction = Turntable_NewAction;        // Switch Old Action
          Turntable_NewAction = DCC_T1CW;                   // Action: Turn Motor M1 1 Step ClockWise
          Turntable_Action = DCC_T1CW;                      // Requested Action = DCC_T1CW
        } // END if

        if (DCC_Accessory[addr].Button == 1)                // Green Button : 1 = TURN 1 Step Counter ClockWise
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin2;  // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_CurrentTrack - 1;
          if (Turntable_NewTrack == 0)                       // From Track 1 to Track 36
          {
            Turntable_NewTrack = maxTrack;                  // Track (maxTrack)
          } // END if
          Turntable_OldAction = Turntable_NewAction;        // Switch Old Action
          Turntable_NewAction = DCC_T1CCW;                  // Action: Turn Motor M1 1 Step Counter ClockWise
          Turntable_Action = DCC_T1CCW;                     // Requested Action = DCC_T1CCW
        } // END if
        break;

      case (228):                                           // DCC Address 228 0 = Direction CW, 1 = Direction CCW
        if (DCC_Accessory[addr].Button == 0)                // Red Button   : 0 = Direction CW
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin1;  // Set Arduino Output Pin
          speedValue = maxSpeed;                            // Positive = Turn ClockWise
          SetDirection();                                   // Determine Turn ClockWise or Counter ClockWise
          Turntable_Action = Turntable_NewAction;           // Requested Action = Depends on SetDirection
        } // END if

        if (DCC_Accessory[addr].Button == 1)                // Green Button : 1 = Direction CCW
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin2;  // Set Arduino Output Pin
          speedValue = -maxSpeed;                           // Negative = Turn Counter ClockWise
          SetDirection();                                   // Determine Turn ClockWise or Counter ClockWise
          Turntable_Action = Turntable_NewAction;           // Requested Action = Depends on SetDirection
        } // END if
        break;

      default:
        if (DCC_Accessory[addr].Button == 0)                // Red Button   : 0 = Goto Track Position0
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin1;  // Set Arduino Output Pin
          Turntable_NewTrack = DCC_Accessory[addr].Position0;
          SetDirection();                                   // Determine Turn ClockWise or Counter ClockWise
          Turntable_Action = Turntable_NewAction;           // Requested Action = Depends on SetDirection
        } // END if

        if (DCC_Accessory[addr].Button == 1)                // Green Button : 1 = Goto Track Position1
        {
          DCC_Action_LED = DCC_Accessory[addr].OutputPin2;  // Set Arduino Output Pin
          Turntable_NewTrack = DCC_Accessory[addr].Position1;
          SetDirection();                                   // Determine Turn ClockWise or Counter ClockWise
          Turntable_Action = Turntable_NewAction;           // Requested Action = Depends on SetDirection
        } // END if
        break;
    }
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("DCC_ACC_Status       --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)DCC_ACC_Status  "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  }

  if ((!DCC_Accessory[addr].Finished) && (millis() > DCC_Accessory[addr].offMilli))
  {
    DCC_Accessory[addr].Finished = 1;
    DCC_Accessory[addr].Active = 0;
  } // END if

  if (++addr >= DCC_Max_Accessories)                        // End loop through DCC Addresses
  {
    addr = 0;
  } // END if
} // END DCC_Accessory_CheckStatus


void Button_CheckStatus()
{
  Button_180.loop();                                        // Check debounce and update the state of the button
  Button_Right.loop();                                      // Check debounce and update the state of the button
  Button_Left.loop();                                       // Check debounce and update the state of the button
  if (Button_Right.isPressed())                             // Button Right Pressed : TURN 1 Step ClockWise
  {
    DCC_Action_LED = 14;                                    // Set Arduino Output Pin 14 = Red LED
    Turntable_NewTrack = Turntable_CurrentTrack + 1;
    if (Turntable_NewTrack > maxTrack)                      // From Track 36 to Track 1
    {
      Turntable_NewTrack = 1;                               // Track (1)
    } // END if
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = Button_T1CW;                      // Action: Turn Motor M1 1 Step ClockWise
    Turntable_Action = Button_T1CW;                         // Requested Action = Button_T1CW
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("BUT_T1CW Status      --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)BUT_T1CW Status "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
    LCDPrintTrackText();
  } // END if

  if (Button_Left.isPressed())                              // Button Left Pressed: TURN 1 Step Counter ClockWise
  {
    DCC_Action_LED = 15;                                    // Set Arduino Output Pin 15 = Green LED
    Turntable_NewTrack = Turntable_CurrentTrack - 1;
    if (Turntable_NewTrack == 0)                            // From Track 1 to Track 36
    {
      Turntable_NewTrack = maxTrack;                        // Track (maxTrack)
    } // END if
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = Button_T1CCW;                     // Action: Turn Motor M1 1 Step Counter ClockWise
    Turntable_Action = Button_T1CCW;                        // Requested Action = Button_T1CCW
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("BUT_T1CCW Status     --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)BUT_T1CCW Status"));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
    LCDPrintTrackText();
  } // END if

  if (Button_180.isPressed())                               // Button T180 Pressed : TURN 180
  {
    DCC_Action_LED = 16;                                    // Set Arduino Output Pin 16 = Yellow LED
    if (Turntable_CurrentTrack <= (maxTrack / 2))
    {
      Turntable_NewTrack = Turntable_CurrentTrack + (maxTrack / 2);
    } // END if
    else
    {
      Turntable_NewTrack = Turntable_CurrentTrack - (maxTrack / 2);
    } // END else
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = Button_T180;                      // Action: Turn Motor M1 (maxTrack / 2) Steps
    Turntable_Action = Button_T180;                         // Requested Action = Button_T180
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("BUT_T180 Status      --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)BUT_T180 Status "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
    LCDPrintTrackText();
  } // END if
} // END Button_CheckStatus


void Turntable_CheckSwitch()                                // From HIGH to LOW = Bridge in Position
{
  Turntable_Switch.loop();                                  // Check debounce and update the state of the Turntable Switch
  if (Turntable_Switch.isPressed())                         // Bridge in Position
  {
        Turntable_OldAction = Turntable_NewAction;          // Switch Old Action
        Turntable_NewAction = POS;                          // Bridge in Position
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckSwitch       --> "));       // Serial print Function
        PrintStatus();                                      // Print Actions and Track Numbers
  } // END if
} // END Turntable_CheckSwitch


void Turntable_StoreTrack()                                 // Store Track Position in EEPROM
{
  digitalWrite(Turntable_Status, HIGH);                     // LED ON = Arduino Onboard LED 13 = Bridge in Position
  digitalWrite(DCC_Action_LED  , LOW);                      // LED OFF
  EEPROM.update(EE_Address, Turntable_CurrentTrack);        // Store Turntable bridge position into EEPROM
} // END Turntable_StoreTrack


void Turntable_CheckPos()                                   // Check if Bridge in Position
{
  if (Turntable_CurrentTrack == Turntable_NewTrack)         // Bridge in Position = Stop Motor M1
  {
    switch (Turntable_OldAction)                            // Check OldAction
    {
      case MCW:                                             // Turntable turning ClockWise
//        for (speedValue; speedValue >= 0; --speedValue)     // Decrease speed to 0 with some delay
//        {
//          delay(3);                                         // Delay to get better bridge to track position
//          Turntable_setM1Speed(speedValue);                 // Motor M1 Set Speed
//        } // END for
        speedValue = 0;                                     // Set speed to 0
        Turntable_setM1Speed(speedValue);                   // Motor M1 Set Speed
        Turntable_OldAction = Turntable_NewAction;          // Switch Old Action
        Turntable_NewAction = STOP;                         // Action: STOP
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckPos MCW      --> "));       // Serial print Function
    //    lcd.setCursor(0, 2);                                // Set cursor to third line and left corner
//                   01234567890123456789                   // Sample text
    //    lcd.print(F("(..)TT_CheckPos MCW "));               // LCD print text
        PrintStatus();                                      // Print Actions and Track Numbers
        break;
      case MCCW:                                            // Turntable turning Counter ClockWise
//        for (speedValue; speedValue <= 0; ++speedValue)     // Decrease speed to 0 with some delay
//        {
//          delay(3);                                         // Delay to get better bridge to track position
//          Turntable_setM1Speed(speedValue);                 // Motor M1 Set Speed
//        } // END for
        speedValue = 0;                                     // Set speed to 0
        Turntable_setM1Speed(speedValue);                   // Motor M1 Set Speed
        Turntable_OldAction = Turntable_NewAction;          // Switch Old Action
        Turntable_NewAction = STOP;                         // Set New Action: STOP
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckPos MCCW     --> "));       // Serial print Function
    //    lcd.setCursor(0, 2);                                // Set cursor to third line and left corner
//                   01234567890123456789                   // Sample text
    //    lcd.print(F("(..)TT_CheckPos MCCW"));               // LCD print text
        PrintStatus();                                      // Print Actions and Track Numbers
        break;
      default:                                              // None of the above
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckPos "));                    // Serial print Function
        Serial.print(Turntable_OldAction);                  // Print OldAction
        Serial.print(F(" --> "));                           // Serial print Function
    //    lcd.setCursor(0, 2);                                // Set cursor to third line and left corner
//                   01234567890123456789                   // Sample text
    //    lcd.print(F("(..)TT_CheckPos     "));               // LCD print text
    //    lcd.setCursor(16, 2);                               // Set cursor to third line and 16th character
    //    lcd.print(Turntable_OldAction);                     // LCD print text
        PrintStatus();                                      // Print Actions and Track Numbers
        break;
    } // END switch
  } // END if
  else                                                      // Bridge NOT in Position = Don't Stop Motor M1
  {
    switch (Turntable_OldAction)                            // Check OldAction
    {
      case MCW:                                             // Turntable turning ClockWise
        digitalWrite(Turntable_Status, LOW);                // LED OFF = Arduino Onboard LED 13 = Bridge in Position
        digitalWrite(DCC_Action_LED  , HIGH);               // LED OFF
        Turntable_OldAction = Turntable_NewAction;          // Switch Old Action
        Turntable_NewAction = MCW;                          // Action: MCW
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckPos MCW      --> "));       // Serial print Function
    //    lcd.setCursor(0, 2);                                // Set cursor to third line and left corner
//                   01234567890123456789                   // Sample text
    //    lcd.print(F("(..)TT_CheckPos MCW "));               // LCD print text
        PrintStatus();                                      // Print Actions and Track Numbers
        break;
      case MCCW:                                            // Turntable turning Counter ClockWise
        digitalWrite(Turntable_Status, LOW);                // LED OFF = Arduino Onboard LED 13 = Bridge in Position
        digitalWrite(DCC_Action_LED  , HIGH);               // LED OFF
        Turntable_OldAction = Turntable_NewAction;          // Switch Old Action
        Turntable_NewAction = MCCW;                         // Set New Action: MCCW
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckPos MCCW     --> "));       // Serial print Function
    //    lcd.setCursor(0, 2);                                // Set cursor to third line and left corner
//                   01234567890123456789                   // Sample text
    //    lcd.print(F("(..)TT_CheckPos MCCW"));               // LCD print text
        PrintStatus();                                      // Print Actions and Track Numbers
        break;
      default:                                              // None of the above
//                      012345678901234567890123456789      // Sample text
        Serial.print(F("TT_CheckPos "));                    // Serial print Function
        Serial.print(Turntable_OldAction);                  // Print OldAction
        Serial.print(F(" --> "));                           // Serial print Function
    //    lcd.setCursor(0, 2);                                // Set cursor to third line and left corner
//                   01234567890123456789                   // Sample text
    //    lcd.print(F("(..)TT_CheckPos     "));               // LCD print text
    //    lcd.setCursor(16, 2);                               // Set cursor to third line and 16th character
    //    lcd.print(Turntable_OldAction);                     // LCD print text
        PrintStatus();                                      // Print Actions and Track Numbers
        break;
    } // END switch
  } // END else
} // END Turntable_CheckPos


void loop()
{
  DCC_Accessory_CheckStatus();                              // Check DCC Accessory Status
  Button_CheckStatus();                                     // Check Button Status
  Turntable_CheckSwitch();                                  // Check Kato Turntable Pin 1

  if ((Turntable_OldAction == STOP) && (Turntable_NewAction == DCC_INPUT))
  {
    Turntable_Init();                                       // Action: Initialize Turntable
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = DCC_INPUT;                        // Action: Initialize Turntable
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)DCC_INPUT        --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)DCC_INPUT       "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == STOP) && (Turntable_NewAction == DCC_CLEAR))
  {
    Turntable_Clear();                                      // Action: Set Turntable Track to 1
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = DCC_CLEAR;                        // Action: Set Turntable Track to 1
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)DCC_CLEAR        --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)DCC_CLEAR       "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == STOP) && (Turntable_NewAction == DCC_END))
  {
    Turntable_End();                                        // End all actions
    Turntable_OldAction = STOP;
    Turntable_NewAction = POS;                              // Bridge in Position
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)DCC_END          --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)DCC_END         "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != DCC_T180) && (Turntable_NewAction == DCC_T180))
  {
    if ((Turntable_CurrentTrack >= 1) && (Turntable_CurrentTrack <= 18))
    {
      speedValue = maxSpeed;                                // Positive = Direction ClockWise
      Turntable_MotorCW();                                  // Motor M1 Forward
      Turntable_OldAction = Turntable_NewAction;            // Switch Old Action
      Turntable_NewAction = MCW;                            // Action: Move Motor M1 ClockWise
    } // END if
    else
    {
      speedValue = -maxSpeed;                               // Negative = Direction Counter ClockWise
      Turntable_MotorCCW();                                 // Motor M1 Backward
      Turntable_OldAction = Turntable_NewAction;            // Switch Old Action
      Turntable_NewAction = MCCW;                           // Action: Move Motor M1 Counter ClockWise
    } // END else
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)DCC_T180         --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)DCC_T180        "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != DCC_T1CW) && (Turntable_NewAction == DCC_T1CW))
  {
    speedValue = maxSpeed;                                  // Positive = Direction ClockWise
    Turntable_MotorCW();                                    // Motor M1 Forward
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = MCW;                              // Action: Move Motor M1 ClockWise
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)Check T1CW       --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)Check T1CW      "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != DCC_T1CCW) && (Turntable_NewAction == DCC_T1CCW))
  {
    speedValue = -maxSpeed;                                 // Negative = Direction Counter ClockWise
    Turntable_MotorCCW();                                   // Motor M1 Reverse
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = MCCW;                             // Action: Move Motor M1 Counter ClockWise
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)Check T1CCW      --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)Check T1CCW     "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != Button_T180) && (Turntable_NewAction == Button_T180))
  {
    if ((Turntable_CurrentTrack >= 1) && (Turntable_CurrentTrack <= 18))
    {
      speedValue = maxSpeed;                                // Positive = Direction ClockWise
      Turntable_MotorCW();                                  // Motor M1 Forward
      Turntable_OldAction = Turntable_NewAction;            // Switch Old Action
      Turntable_NewAction = MCW;                            // Action: Move Motor M1 ClockWise
    } // END if
    else
    {
      speedValue = -maxSpeed;                               // Negative = Direction Counter ClockWise
      Turntable_MotorCCW();                                 // Motor M1 Backward
      Turntable_OldAction = Turntable_NewAction;            // Switch Old Action
      Turntable_NewAction = MCCW;                           // Action: Move Motor M1 Counter ClockWise
    } // END else
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)BUT_T180         --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)BUT_T180        "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != Button_T1CW) && (Turntable_NewAction == Button_T1CW))
  {
    speedValue = maxSpeed;                                  // Positive = Direction ClockWise
    Turntable_MotorCW();                                    // Motor M1 Forward
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = MCW;                              // Action: Move Motor M1 ClockWise
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)BUT_T1CW         --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)BUT_T1CW        "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != Button_T1CCW) && (Turntable_NewAction == Button_T1CCW))
  {
    speedValue = -maxSpeed;                                 // Negative = Direction Counter ClockWise
    Turntable_MotorCCW();                                   // Motor M1 Reverse
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = MCCW;                             // Action: Move Motor M1 Counter ClockWise
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)BUT_T1CCW        --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)BUT_T1CCW       "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != TCW) && (Turntable_NewAction == TCW))
  {
    speedValue = maxSpeed;                                  // Positive = Direction ClockWise
    Turntable_MotorCW();                                    // Motor M1 Forward
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = MCW;                              // Action: Move Motor M1 ClockWise
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)Check TCW        --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)Check TCW       "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != TCCW) && (Turntable_NewAction == TCCW))
  {
    speedValue = -maxSpeed;                                 // Negative = Direction Counter ClockWise
    Turntable_MotorCCW();                                   // Motor M1 Reverse
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = MCCW;                             // Action: Move Motor M1 Counter ClockWise
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)Check TCCW       --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)Check TCCW      "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == MCW) && (Turntable_NewAction == POS)) // Move ClockWise and Turntable in Position
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack + 1;
    if (Turntable_CurrentTrack > maxTrack)                  // From Track 36 to Track 1
    {
      Turntable_CurrentTrack = 1;                           // Track (1)
    } // END if
    Turntable_CheckPos();                                   // Check if Bridge in Position
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)T+1 - Check MCW  --> "));           // Serial print Function
////    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)T+1 - Check MCW "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == MCCW) && (Turntable_NewAction == POS)) // Move Counter ClockWise and Turntable in Position
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack - 1;
    if (Turntable_CurrentTrack == 0)                        // From Track 1 to Track 36
    {
      Turntable_CurrentTrack = maxTrack;                    // Track (maxTrack)
    } // END if
    Turntable_CheckPos();                                   // Check if Bridge in Position
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)T-1 - Check MCCW --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)T-1 - Check MCCW"));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == POS) && (Turntable_NewAction == STOP)) // STOP
  {
    Turntable_StoreTrack();                                 // Store Track Position in EEPROM
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = STOP;                             // Action: STOP
//                  012345678901234567890123456789          // Sample text
    Serial.print(F("(..)STOP             --> "));           // Serial print Function
//    lcd.setCursor(0, 2);                                    // Set cursor to third line and left corner
//               01234567890123456789                       // Sample text
//    lcd.print(F("(..)STOP            "));                   // LCD print text
    PrintStatus();                                          // Print Actions and Track Numbers
  } // END if
} // END loop


void DCC_Accessory_LED_OFF()                                // All LEDs OFF
{
  for (int i = 0; i < DCC_Max_Accessories; i++)
  {
    digitalWrite(DCC_Accessory[i].OutputPin1, LOW);         // LED OFF
    digitalWrite(DCC_Accessory[i].OutputPin2, LOW);         // LED OFF
  } // END for
} // END DCC_Accessory_LED_OFF


void Turntable_setM1Speed(int speed)                        // Motor M1 Set Speed
{
  if (speed == 0)                                           // Motor M1 Stop
  {
    digitalWrite(Turntable_MotorM1, LOW);
    digitalWrite(Turntable_MotorM2, LOW);
  } // END if
  if (speed > 0)                                            // Motor M1 Forward
  {
    digitalWrite(Turntable_MotorM2, LOW);
    analogWrite(Turntable_MotorM1, speed);                  // PWM on Motor M1
  } // END if
  if (speed < 0)                                            // Motor M1 Reverse
  {
    digitalWrite(Turntable_MotorM1, LOW);
    analogWrite(Turntable_MotorM2, speed);                  // PWM on Motor M2
  } // END if
  LCDPrintTrackText();
  
  LCDPrintTrackStatus();
} // END Turntable_setM1Speed


void Turntable_Init()                                       // Start Initialize Turntable Procedure
{
  digitalWrite(Turntable_Status, LOW);                      // LED OFF = Arduino Onboard LED 13 = Bridge in Position
  digitalWrite(DCC_Action_LED  , HIGH);                     // LED ON
} // END Turntable_Init


void Turntable_Clear()                                      // Simple Blink Action to comfirm DCC_CLEAR
{
  digitalWrite(DCC_Action_LED  , LOW);                      // LED OFF
  delay(200);
  digitalWrite(DCC_Action_LED  , HIGH);                     // LED ON
  delay(200);
  digitalWrite(DCC_Action_LED  , LOW);                      // LED OFF
  delay(200);
  digitalWrite(DCC_Action_LED  , HIGH);                     // LED ON
  delay(200);
  digitalWrite(DCC_Action_LED  , LOW);                      // LED OFF
  delay(1000);
  digitalWrite(DCC_Action_LED  , HIGH);                     // LED ON
} // END Turntable_Clear


void Turntable_End()                                        // End Initialize Turntable Procedure
{
  digitalWrite(DCC_Action_LED  , LOW);                      // LED OFF
} // END Turntable_End


void Turntable_MotorCW()                                    // Motor M1 Forward
{
  digitalWrite(Turntable_Status, LOW);                      // LED OFF = Arduino Onboard LED 13 = Bridge in Position
  digitalWrite(DCC_Action_LED  , HIGH);                     // LED ON
  Turntable_setM1Speed(speedValue);                         // Motor M1 Speed value
  Turntable_TurnStart = millis();                           // Time when turn starts
} // END Turntable_MotorCW


void Turntable_MotorCCW()                                   // Motor M1 Reverse
{
  digitalWrite(Turntable_Status, LOW);                      // LED OFF = Arduino Onboard LED 13 = Bridge in Position
  digitalWrite(DCC_Action_LED  , HIGH);                     // LED ON
  Turntable_setM1Speed(speedValue);                         // Motor M1 Speed value
  Turntable_TurnStart = millis();                           // Time when turn starts
} // END Turntable_MotorCCW


void SetDirection()
{
//  if (Turntable_NewTrack <= (Turntable_CurrentTrack + (maxTrack / 2)))
  if (speedValue > 0)
  {
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = TCW;                              // Action: Turn Motor M1 ClockWise
  } // END if
  else
  {
    Turntable_OldAction = Turntable_NewAction;              // Switch Old Action
    Turntable_NewAction = TCCW;                             // Action: Turn Motor M1 Counter ClockWise
  } // END else
  Serial.print(F("SetDirection         --> "));             // Serial print Function
  PrintStatus();                                            // Print Actions and Track Numbers
} // END SetDirection


void PrintStatus()
{
  Serial.print(Turntable_States[Turntable_Action]);         // Serial print action
  Serial.print(F(": Old: "));                               // Serial print text
  Serial.print(Turntable_States[Turntable_OldAction]);      // Serial print action
  Serial.print(F(", New: "));                               // Serial print text
  Serial.print(Turntable_States[Turntable_NewAction]);      // Serial print action
  Serial.print(F(", Current: "));                           // Serial print text
  Serial.print(Turntable_CurrentTrack);                     // Serial print value
  Serial.print(F(", NewTrack: "));                          // Serial print text
  Serial.print(Turntable_NewTrack);                         // Serial print value
  Serial.print(F(", DCC_Action_LED: "));                    // Serial print text
  Serial.print(DCC_Action_LED);                             // Serial print value
  Serial.print(F(", Speed: "));                             // Serial print text
  Serial.print(speedValue);                                 // Serial print value
  Serial.println();                                         // Serial print new line
} // END PrintStatus


void LCDPrintTrackStatus()
{
  if (Turntable_CurrentTrack < 10)
  {
    lcd.setCursor(8, 3);                                    // Set cursor to first line and 9th character
  } // END if
  else
  {
    lcd.setCursor(7, 3);                                    // Set cursor to first line and 8th character
  } // END else
  lcd.print(Turntable_CurrentTrack);                        // LCD print value
  if (Turntable_NewTrack < 10)
  {
    lcd.setCursor(14, 3);                                   // Set cursor to first line and 15th character
  } // END if
  else
  {
    lcd.setCursor(13, 3);                                   // Set cursor to first line and 14th character
  } // END else
  lcd.print(Turntable_NewTrack);                            // LCD print value
  lcd.setCursor(16, 3);                                     // Set cursor to first line and 17th character
  if (speedValue == 0)
  {
  lcd.print(F("-  "));                                      // LCD print text
  }
  if (speedValue < 0)
  {
  lcd.print(F("<"));                                        // LCD print text
  }  
  if (speedValue > 0)
  {
  lcd.print(F(">"));                                        // LCD print text
  }
  lcd.setCursor(17, 3);                                     // Set cursor to first line and 18th character
  if (abs(speedValue) < 100)
  {
    lcd.setCursor(18, 3);                                   // Set cursor to first line and 19th character
  }
  if (abs(speedValue) < 10)
  {
    lcd.setCursor(19, 3);                                   // Set cursor to first line and 20th character
  }  
  lcd.print(abs(speedValue));                               // LCD print value
} // END LCDPrintTrackStatus


void LCDPrintTrackText()
{
  lcd.setCursor(0, 3);                                      // Set cursor to fourth line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("Track:    to        "));                     // LCD print text
} // END LCDPrintTrackText


void LCDPrintAction()
{
  lcd.setCursor(10, 1);                                     // Set cursor to second line and 10th character
  lcd.print(Turntable_States[Turntable_Action]);            // LCD print action
  lcd.setCursor(10, 3);                                     // Set cursor to fourth line and 10th character
  lcd.print(Turntable_States[Turntable_NewAction]);         // LCD print text

  lcd.setCursor(0, 3);                                      // Set cursor to fourth line and left corner
  lcd.print(Turntable_States[Turntable_OldAction]);         // LCD print text
  lcd.setCursor(10, 3);                                     // Set cursor to fourth line and 10th character
  lcd.print(Turntable_States[Turntable_NewAction]);         // LCD print text
} // END LCDPrintAction


void LCDPrintClear()
{
  lcd.setCursor(0, 0);                                      // Set cursor to first line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("                    "));                     // Clear text
  lcd.setCursor(0, 1);                                      // Set cursor to second line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("                    "));                     // Clear text
  lcd.setCursor(0, 2);                                      // Set cursor to third line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("                    "));                     // Clear text
  lcd.setCursor(0, 3);                                      // Set cursor to fourth line and left corner
//             01234567890123456789                         // Sample text
  lcd.print(F("                    "));                     // Clear text
}
