//-----------------------------------------------------------------------------// DCCNext-Controlled-Kato-Turntable_v2.07
// Sketch uses 16034 bytes (49%) of program storage space. Maximum is 32256 bytes.
// Global variables use 1566 bytes (76%) of dynamic memory, leaving 482 bytes for local variables. Maximum is 2048 bytes.
// Low memory available, stability problems may occur.
//-----------------------------------------------------------------------------//
// The programmed tracks 1-36, correspond to adresses 401 to 436.
// Red = Bridge hut end of turntable will rotate to selected track.
// Green = Opposite bridge hut end will rotate to selected track.
// Address 400 = 180° bridge rotation, Red = Clockwise (CW), Green = Counter Clockwise (CCW).
#include <EEPROM.h>                                                            // Standard Arduino EEPROM library
#include <DCC_Decoder.h>                                                       // Use Manage Libraries to add: NmraDcc -- https://github.com/MynaBay/DCC_Decoder
#include <ezButton.h>                                                          // Use Manage Libraries to add: ezButton -- https://github.com/ArduinoGetStarted/button
#include <LiquidCrystal_I2C.h>                                                 // Use Manage Libraries to add: LiquidCrystal I2C -- https://github.com/johnrickman/LiquidCrystal_I2C
#define kDCC_INTERRUPT                            0                            // DCC Interrupt 0
#define DCC_Address_Offset                        1                            // Default = 1, for Multimaus = 4
#define DCC_Max_Accessories                      19                            // Total Number of DCC Accessory Decoder 0..18 = Addresses = 400 - 436
#define DCC_Interrupt                             2                            // Arduino Input  Pin  2 = DCC signal = Interrupt 0
const uint8_t  Turntable_MotorM1            =     5;                           // Arduino Output Pin  5 = Turntable Motor     = Turntable Cable Pin 3
const uint8_t  Turntable_MotorM2            =     6;                           // Arduino Output Pin  6 = Turntable Motor     = Turntable Cable Pin 4
const uint8_t  BridgeRelayL1                =     7;                           // Arduino Output Pin  7 = ULN2803A Pin 1+2    = Bridge Relay L1-
const uint8_t  BridgeRelayL2                =     8;                           // Arduino Output Pin  8 = ULN2803A Pin 3+4    = Bridge Relay L2-
const uint8_t  Turntable_LockL2             =     9;                           // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
const uint8_t  Turntable_LockL1             =    10;                           // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
const uint8_t  WatchdogLED                  =    13;                           // Arduino Output Pin 13 = DCCNext Red LED     = Watchdog Blink
const uint8_t  RedLED                       =    14;                           // Arduino Output Pin 14 = Red LED             = Function Red
const uint8_t  GreenLED                     =    15;                           // Arduino Output Pin 15 = Green LED           = Function Green
const uint8_t  YellowLED                    =    16;                           // Arduino Output Pin 16 = Yellow LED          = Turn 180
const uint8_t  Turntable_StatusLED          =    17;                           // Arduino Output Pin 17 = Blue LED            = Bridge in Position
                                                                               // Arduino VCC Pin       = Bridge Relay L1+
                                                                               // Arduino VCC Pin       = Bridge Relay L2+
                                                                               // Arduino Ground Pin GND                      = Turntable Cable Pin 2
ezButton       Turntable_Switch(3);                                            // Arduino Input  Pin  3 = Turntable Trigger   = Turntable Cable Pin 1
ezButton       Button_T180(4);                                                 // Arduino Input  Pin  4 = Button Turn 180     = Turn 180
ezButton       Button_Right(11);                                               // Arduino Input  Pin 11 = Button Turn Right   = Turn 1 Step ClockWise
ezButton       Button_Left(12);                                                // Arduino Input  Pin 12 = Button Turn Left    = Turn 1 Step Counter ClockWise
const uint32_t Turntable_SwitchDebounceTime =    30;                           // Turntable Switch debounce time in ms
const uint32_t Button_T180DebounceTime      =    50;                           // Button T180 debounce time in ms
const uint32_t Button_RightDebounceTime     =    50;                           // Button Right debounce time in ms
const uint32_t Button_LeftDebounceTime      =    50;                           // Button Left debounce time in ms
uint32_t       Button_T180_PressedTime      =     0;                           // Button T180 pressed time in ms
uint32_t       Button_T180_ReleasedTime     =     0;                           // Button T180 released time in ms
uint32_t       Button_T180_PressTime        =     0;                           // Button T180 press time in ms
const uint32_t Button_T180_ShortPressTime   =  1000;                           // Button T180 short press time in ms
const uint32_t Button_T180_LongPressTime    =  3000;                           // Button T180 long press time in ms
boolean        Button_T180_CurrentState     =  HIGH;                           // Button T180 current state (Released = HIGH)
boolean        Button_T180_LastState        =  HIGH;                           // Button T180 last state (Released = HIGH)
boolean        Button_T180_IsPressed        = false;                           // Button T180 pressed
boolean        Button_T180_IsPressing       = false;                           // Button T180 pressing
boolean        Button_T180_ShortPressed     = false;                           // Button T180 short pressed
boolean        Button_T180_LongPressed      = false;                           // Button T180 long pressed
uint32_t       Button_Right_PressedTime     =     0;                           // Button Right pressed time in ms
uint32_t       Button_Right_ReleasedTime    =     0;                           // Button Right released time in ms
uint32_t       Button_Right_PressTime       =     0;                           // Button Right press time in ms
const uint32_t Button_Right_ShortPressTime  =  1000;                           // Button Right short press time in ms
const uint32_t Button_Right_LongPressTime   =  3000;                           // Button Right long press time in ms
boolean        Button_Right_CurrentState    =  HIGH;                           // Button Right current state (Released = HIGH)
boolean        Button_Right_LastState       =  HIGH;                           // Button Right last state (Released = HIGH)
boolean        Button_Right_IsPressed       = false;                           // Button Right pressed
boolean        Button_Right_IsPressing      = false;                           // Button Right pressing
boolean        Button_Right_ShortPressed    = false;                           // Button Right short pressed
boolean        Button_Right_LongPressed     = false;                           // Button Right long pressed
uint32_t       Button_Left_PressedTime      =     0;                           // Button Left pressed time in ms
uint32_t       Button_Left_ReleasedTime     =     0;                           // Button Left released time in ms
uint32_t       Button_Left_PressTime        =     0;                           // Button Left press time in ms
const uint32_t Button_Left_ShortPressTime   =  1000;                           // Button Left short press time in ms
const uint32_t Button_Left_LongPressTime    =  3000;                           // Button Left long press time in ms
boolean        Button_Left_CurrentState     =  HIGH;                           // Button Left current state (Released = HIGH)
boolean        Button_Left_LastState        =  HIGH;                           // Button Left last state (Released = HIGH)
boolean        Button_Left_IsPressed        = false;                           // Button Left pressed
boolean        Button_Left_IsPressing       = false;                           // Button Left pressing
boolean        Button_Left_ShortPressed     = false;                           // Button Left short pressed
boolean        Button_Left_LongPressed      = false;                           // Button Left long pressed
uint8_t        DCC_Action_LED               =     0;                           // Pin Number will change by DCC command (pin 14 = Red , 15 = Green, 16 = Blue)
uint8_t        Turntable_CurrentTrack       =     0;                           // Turntable Current Track
uint8_t        Turntable_NewTrack           =     0;                           // Turntable New Track
const uint8_t  EE_Address                   =     0;                           // EEPROM Address Turntable Bridge Position
int            speedValue                   =     0;                           // Turntable Motor Speed = maxSpeed or -maxSpeed
const uint8_t  maxSpeed                     =   100;                           // Speed between -255 = Reversed to 255 = Forward (-5 to +5 VDC)
const uint8_t  maxTrack                     =    36;                           // Total number of turntable tracks
boolean        DCC_ReverseTrack[37];                                           // Status will change by DCC command (false = Normal, true = Reversed) 
// Note: Size of DCC_ReverseTrack must be maxTrack + 1 !!                      // --> [0..maxTrack] = maxTrack + 1 records !!                                                                               // Note: Size of DCC_ReverseTrack must be the same as maxTrack !!
const uint32_t WatchdogInterval             =   250;                           // Watchdog blink interval in ms
uint32_t       WatchdogMillis               =     0;                           // Last time Watchdog LED was updated
uint8_t        WatchdogCounter              =     0;                           // Watchdog Counter
boolean        WatchdogState                =   LOW;                           // Watchdog LED state
boolean        BridgeRelayActive            = false;                           // Bridge Relay Not Active
uint32_t       BridgeRelayMaxMillis         =     0;                           // Bridge Relay Active Timer
const uint32_t BridgeRelayPulsTime          =   250;                           // Bridge Relay Pulse Time in ms
uint32_t       TurntableLockMillis          =     0;                           // Turntable Lock Active Timer
const uint32_t TurntableLockInterval        =   500;                           // Turntable Lock Pulse Time in ms
boolean        TurntableLockActive          = false;                           // Turntable Lock Not Active
uint32_t       TurntableWaitMillis          =     0;                           // Turntable Wait Before Turn Active Timer
const uint32_t TurntableWaitInterval        =  2000;                           // Turntable Wait Before Turn Time in ms
boolean        TurntableWaitActive          = false;                           // Turntable Wait Before Turn Not Active
uint8_t        Track1                       =     0;                           // Temp variables
uint8_t        Track2                       =     0;                           // Temp variables

const char* Turntable_States[] =                                               // Possible Turntable States
{//012345678
  "POS      ",                                                                 // Bridge in Position
  "STOP     ",                                                                 // Stop Turning
  "UNLOCKING",                                                                 // Unlocking Turntable Lock
  "UNLOCKED ",                                                                 // Turntable Lock Unlocked
  "TURN_WAIT",                                                                 // Turntable Wait Before Turn
  "SETDIR   ",                                                                 // Set Direction
  "TCW      ",                                                                 // Turn ClockWise
  "TCCW     ",                                                                 // Turn Counter ClockWise
  "MCW      ",                                                                 // Motor ClockWise
  "MCCW     ",                                                                 // Motor Counter ClockWise
  "DCC_END  ",                                                                 // DCC Command END                           - Button 225: 0 = OFF (Red)
  "DCC_INPUT",                                                                 // DCC Command INPUT                         - Button 225: 1 = ON  (Green)
  "DCC_CLEAR",                                                                 // DCC Command CLEAR                         - Button 226: 0 = OFF (Red)
  "DCC_TURN ",                                                                 // DCC Command Turn                          - Button 401..436
  "DCC_T180 ",                                                                 // DCC Command Turn 180                      - Button 226: 1 = ON  (Green)
  "BUT_T180 ",                                                                 // Button T180  (Shortpress) = Turn 180
  "BUT_STORE",                                                                 // Button T180  (Longpress)  = Store current position as track 1
  "BUT_T1CW ",                                                                 // Button Right (Shortpress) = Turn 1 Step ClockWise
  "BUT_RLOCK",                                                                 // Button Right (Longpress)  = Reset Turntable Lock
  "BUT_T1CCW",                                                                 // Button Left  (Shortpress) = Turn 1 Step Counter ClockWise
  "BUT_SLOCK"                                                                  // Button Left  (Longpress)  = Set Turntable Lock
}; // END const

enum Turntable_NewActions:uint8_t                                              // Possible Turntable Actions
{
  POS       ,                                                                  // Bridge in Position
  STOP      ,                                                                  // Stop Turning
  UNLOCKING ,                                                                  // Unlocking Turntable Lock
  UNLOCKED  ,                                                                  // Turntable Lock Unlocked
  TURN_WAIT ,                                                                  // Turntable Wait Before Turn
  SETDIR    ,                                                                  // Set Direction
  TCW       ,                                                                  // Turn ClockWise
  TCCW      ,                                                                  // Turn Counter ClockWise
  MCW       ,                                                                  // Motor ClockWise
  MCCW      ,                                                                  // Motor Counter ClockWise
  DCC_END   ,                                                                  // DCC Command END
  DCC_INPUT ,                                                                  // DCC Command INPUT
  DCC_CLEAR ,                                                                  // DCC Command CLEAR
  DCC_TURN  ,                                                                  // DCC Command Turn
  DCC_T180  ,                                                                  // DCC Command Turn 180
  BUT_T180  ,                                                                  // Button T180  (Shortpress) = Turn 180
  BUT_STORE ,                                                                  // Button T180  (Longpress)  = Store current position as track 1
  BUT_T1CW  ,                                                                  // Button Right (Shortpress) = Turn 1 Step ClockWise
  BUT_RLOCK ,                                                                  // Button Right (Longpress)  = Reset Turntable Lock
  BUT_T1CCW ,                                                                  // Button Left  (Shortpress) = Turn 1 Step Counter ClockWise
  BUT_SLOCK                                                                    // Button Left  (Longpress)  = Set Turntable Lock
}; // END enum

uint8_t Directions[4][4] =                                                     // ToZone (vertical) - FromZone (
{
  { 0, 1, 1, 0 },
  { 0, 0, 1, 0 },
  { 0, 0, 0, 1 },
  { 1, 0, 0, 0 }
};

enum Turntable_NewActions Turntable_OldAction = STOP;                          // Stores Turntable Previous Action
enum Turntable_NewActions Turntable_NewAction = STOP;                          // Stores Turntable New Action
enum Turntable_NewActions Turntable_Action    = STOP;                          // Stores Turntable Requested Action

typedef struct                                                                 // Begin DCC Accessory Structure
{
  int               Address;                                                   // DCC Address to respond to
  uint8_t           Button;                                                    // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  uint8_t           Position1;                                                 // Turntable Position1
  uint8_t           Position2;                                                 // Turntable Position2
  uint8_t           OutputPin1;                                                // Arduino Output Pin 1
  uint8_t           OutputPin2;                                                // Arduino Output Pin 2
  boolean           ReverseTrack1;                                             // Reverse Track Power: 0 = Normal, 1 = Reversed
  boolean           ReverseTrack2;                                             // Reverse Track Power: 0 = Normal, 1 = Reversed
  boolean           Finished;                                                  // Command Busy = 0 or Finished = 1 (Ready for next command)
  boolean           Active;                                                    // Command Not Active = 0, Active = 1
  unsigned long     durationMilli;                                             // Pulse Time in ms
  unsigned long     offMilli;                                                  // For internal use  // Do not change this value
} // END typedef
DCC_Accessory_Structure;                                                       // End DCC Accessory Structure

DCC_Accessory_Structure DCC_Accessory[DCC_Max_Accessories];                    // Define DCC_Accessory as DCC Accessory Structure
LiquidCrystal_I2C lcd(0x27, 20, 4);                                            // I2C Liquid Crystal Display on Address 0x27 with 20 characters by 4 rows


//-----------------------------------------------------------------------------//
void setup()                                                                   // Arduino Setup
{
  Serial.begin(115200);
  Serial.println(F("DCCNext-Controlled-Kato-Turntable_v2.07 -- (c)JMRRvS 2021-01-13"));
                                                                               // Serial print loaded sketch
  lcd.init();                                                                  // Initialise LCD
  lcd.backlight();                                                             // Switch backlight ON
  lcd.setCursor(0, 0);                                                         // Set cursor to first line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("DCCNext Controlled  "));                                        // LCD print text
  lcd.setCursor(0, 1);                                                         // Set cursor to second line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("Kato Turntable v2.07"));                                        // LCD print text
  lcd.setCursor(0, 2);                                                         // Set cursor to third line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("--------------------"));                                        // LCD print text
  Turntable_Switch.setDebounceTime(Turntable_SwitchDebounceTime);              // Set Debounce Time
  Button_T180.setDebounceTime(Button_T180DebounceTime);                        // Set Debounce Time
  Button_Right.setDebounceTime(Button_RightDebounceTime);                      // Set Debounce Time
  Button_Left.setDebounceTime(Button_LeftDebounceTime);                        // Set Debounce Time
  pinMode(DCC_Interrupt      , INPUT_PULLUP);                                  // Arduino Input Pin   2 = DCC signal = Interrupt 0
  // pinMode defined with ezButton function                                    // Arduino Input Pin   3 = Turntable Trigger   = Turntable Cable Pin 1
                                                                               // Arduino Ground Pin GND                      = Turntable Cable Pin 2
  // pinMode defined with ezButton function                                    // Arduino Input  Pin  4 = Button Turn 180     = Turn 180
  pinMode(Turntable_MotorM1  , OUTPUT);                                        // Arduino Output Pin  5 = Turntable Motor     = Turntable Cable Pin 3
  pinMode(Turntable_MotorM2  , OUTPUT);                                        // Arduino Output Pin  6 = Turntable Motor     = Turntable Cable Pin 4
  pinMode(BridgeRelayL1      , OUTPUT);                                        // Arduino Output Pin  7 = ULN2803A Pin 1+2    = Bridge Relay L1-
  pinMode(BridgeRelayL2      , OUTPUT);                                        // Arduino Output Pin  8 = ULN2803A Pin 3+4    = Bridge Relay L2-
  pinMode(Turntable_LockL2   , OUTPUT);                                        // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  pinMode(Turntable_LockL1   , OUTPUT);                                        // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  // pinMode defined with ezButton function                                    // Arduino Input  Pin 11 = Button Turn Right   = Turn 1 Step ClockWise
  // pinMode defined with ezButton function                                    // Arduino Input  Pin 12 = Button Turn Left    = Turn 1 Step Counter ClockWise
  pinMode(WatchdogLED        , OUTPUT);                                        // Arduino Output Pin 13 = DCCNext Red LED     = Watchdog Blink
  pinMode(RedLED             , OUTPUT);                                        // Arduino Output Pin 14 = Red LED             = Function Red
  pinMode(GreenLED           , OUTPUT);                                        // Arduino Output Pin 15 = Green LED           = Function Green
  pinMode(YellowLED          , OUTPUT);                                        // Arduino Output Pin 16 = Yellow LED          = TURN 180
  pinMode(Turntable_StatusLED, OUTPUT);                                        // Arduino Output Pin 17 = Blue LED            = Bridge in Position
  digitalWrite(Turntable_MotorM1, LOW);                                        // Arduino Output Pin  5 = Turntable Motor     = Turntable Cable Pin 3
  digitalWrite(Turntable_MotorM2, LOW);                                        // Arduino Output Pin  6 = Turntable Motor     = Turntable Cable Pin 4
  digitalWrite(BridgeRelayL1    , LOW);                                        // Arduino Output Pin  7 = ULN2803A Pin 1+2    = Bridge Relay L1-
  digitalWrite(BridgeRelayL2    , LOW);                                        // Arduino Output Pin  8 = ULN2803A Pin 3+4    = Bridge Relay L2-
  digitalWrite(Turntable_LockL2 , LOW);                                        // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  digitalWrite(Turntable_LockL1 , LOW);                                        // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5

  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC_Accessory_ConfigureDecoderFunctions();
  DCC.SetupDecoder( 0x00, 0x00, kDCC_INTERRUPT );
  for (uint8_t AccDec = 0; AccDec < DCC_Max_Accessories; AccDec++)             // Begin loop through DCC Accessory Decoders
  {
    DCC_Accessory[AccDec].Button = 0;                                          // Switch OFF all DCC Accessory Decoders
  } // END for

  DCC_Action_LED_Startup();                                                    // All DCC Action LEDs ON and OFF  
  Turntable_CurrentTrack = EEPROM.read(EE_Address);                            // Read Turntable Bridge Position from EEPROM
  if ((Turntable_CurrentTrack < 1) || (Turntable_CurrentTrack > maxTrack))
  {
    Turntable_CurrentTrack = 0;                                                // Reset CurrentTrack if EEPROM value is out of range
    Turntable_NewTrack = 0;                                                    // Reset NewTrack if EEPROM value is out of range
    digitalWrite(Turntable_StatusLED, LOW);                                    // Set Arduino Output Pin 17 = Blue LED = Bridge in Position OFF
    //                0123456789012345678901234                                // Sample text
    Serial.println(F("EEPROM status unknown.   "));                            // Serial print text
  } // END if
  else
  {
    Turntable_NewTrack = Turntable_CurrentTrack;                               // Set new track to current track
    digitalWrite(Turntable_StatusLED, HIGH);                                   // Set Arduino Output Pin 17 = Blue LED = Bridge in Position ON
    BridgeRelayInit();                                                         // Initialize Reverse Tracks
    BridgeRelayActive = true;                                                  // Activate Bridge Relay Set
    BridgeRelaySet();                                                          // Set Bride Relay Status = ULN2803A
  } // END else
  TurntableLockOff();                                                          // Disable Outputs to L293D
  LCDPrintTrackText();                                                         // LCD print text
  LCDPrintTrackStatus();                                                       // LCD print text
} // END setup

//-----------------------------------------------------------------------------//
void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data)
{
  address -= 1;
  address *= 4;
  address += DCC_Address_Offset;                                               // Default = 1, for Multimaus = 4
  address += (data & 0x06) >> 1;                                               // Convert NMRA packet address format to human address
  boolean output = (data & 0x01) ? 1 : 0;                                      // Red = 0, Green = 1
  for (uint8_t AccDec = 0; AccDec < DCC_Max_Accessories; AccDec++)             // Begin loop through DCC Accessory Decoders
  {
    if (address == DCC_Accessory[AccDec].Address)
    {
      DCC_Accessory[AccDec].Active = 1;                                        // DCC Accessory Active
      if (output)
      {
        DCC_Accessory[AccDec].Button = 1;                                      // Green Button
      } // END if
      else
      {
        DCC_Accessory[AccDec].Button = 0;                                      // Red Button
      } // END else
    } // END if
  } // END for
} // END BasicAccDecoderPacket_Handler

//-----------------------------------------------------------------------------//
void DCC_Accessory_ConfigureDecoderFunctions()
{
  DCC_Accessory[0].Address        =   400;                                     // DCC Address 400 0 = Turn 180 CW , 1 = Turn 180 CCW
  DCC_Accessory[0].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[0].Position1      =     0;                                     // Turn 180 CW
  DCC_Accessory[0].Position2      =     0;                                     // Turn 180 CCW
  DCC_Accessory[0].OutputPin1     =    16;                                     // Arduino Output Pin 16 = Yellow LED
  DCC_Accessory[0].OutputPin2     =    16;                                     // Arduino Output Pin 16 = Yellow LED
  DCC_Accessory[0].ReverseTrack1  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[0].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[0].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[0].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[0].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[1].Address        =   401;                                     // DCC Address 401
  DCC_Accessory[1].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[1].Position1      =     1;                                     // Goto Track # CCW
  DCC_Accessory[1].Position2      =    19;                                     // Goto Track # CW
  DCC_Accessory[1].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[1].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[1].ReverseTrack1  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[1].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[1].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[1].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[1].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[2].Address        =   402;                                     // DCC Address
  DCC_Accessory[2].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[2].Position1      =     2;                                     // Goto Track # CCW
  DCC_Accessory[2].Position2      =    20;                                     // Goto Track # CW
  DCC_Accessory[2].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[2].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[2].ReverseTrack1  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[2].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[2].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[2].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[2].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[3].Address        =   403;                                     // DCC Address
  DCC_Accessory[3].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[3].Position1      =     3;                                     // Goto Track # CCW
  DCC_Accessory[3].Position2      =    21;                                     // Goto Track # CW
  DCC_Accessory[3].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[3].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[3].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[3].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[3].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[3].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[3].durationMilli  =   250;                                     // Pulse Time in ms
                   
  DCC_Accessory[4].Address        =   404;                                     // DCC Address
  DCC_Accessory[4].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[4].Position1      =     4;                                     // Goto Track # CCW
  DCC_Accessory[4].Position2      =    22;                                     // Goto Track # CW
  DCC_Accessory[4].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[4].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[4].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[4].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[4].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[4].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[4].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[5].Address        =   405;                                     // DCC Address
  DCC_Accessory[5].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[5].Position1      =     5;                                     // Goto Track # CCW
  DCC_Accessory[5].Position2      =    23;                                     // Goto Track # CW
  DCC_Accessory[5].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[5].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[5].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[5].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[5].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[5].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[5].durationMilli  =   250;                                     // Pulse Time in ms
                   
  DCC_Accessory[6].Address        =   406;                                     // DCC Address
  DCC_Accessory[6].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[6].Position1      =     6;                                     // Goto Track # CCW
  DCC_Accessory[6].Position2      =    24;                                     // Goto Track # CW
  DCC_Accessory[6].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[6].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[6].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[6].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[6].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[6].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[6].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[7].Address        =   407;                                     // DCC Address
  DCC_Accessory[7].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[7].Position1      =     7;                                     // Goto Track # CCW
  DCC_Accessory[7].Position2      =    25;                                     // Goto Track # CW
  DCC_Accessory[7].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[7].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[7].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[7].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[7].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[7].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[7].durationMilli  =   250;                                     // Pulse Time in ms
                   
  DCC_Accessory[8].Address        =   408;                                     // DCC Address
  DCC_Accessory[8].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[8].Position1      =     8;                                     // Goto Track # CCW
  DCC_Accessory[8].Position2      =    26;                                     // Goto Track # CW
  DCC_Accessory[8].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[8].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[8].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[8].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[8].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[8].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[8].durationMilli  =   250;                                     // Pulse Time in ms
                   
  DCC_Accessory[9].Address        =   409;                                     // DCC Address
  DCC_Accessory[9].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[9].Position1      =     9;                                     // Goto Track # CCW
  DCC_Accessory[9].Position2      =    27;                                     // Goto Track # CW
  DCC_Accessory[9].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[9].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[9].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[9].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[9].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[9].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[9].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[10].Address        =  410;                                     // DCC Address
  DCC_Accessory[10].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[10].Position1      =   10;                                     // Goto Track # CCW
  DCC_Accessory[10].Position2      =   28;                                     // Goto Track # CW
  DCC_Accessory[10].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[10].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[10].ReverseTrack1  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[10].ReverseTrack2  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[10].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[10].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[10].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[11].Address        =  411;                                     // DCC Address
  DCC_Accessory[11].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[11].Position1      =   11;                                     // Goto Track # CCW
  DCC_Accessory[11].Position2      =   29;                                     // Goto Track # CW
  DCC_Accessory[11].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[11].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[11].ReverseTrack1  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[11].ReverseTrack2  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[11].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[11].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[11].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[12].Address        =  412;                                     // DCC Address
  DCC_Accessory[12].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[12].Position1      =   12;                                     // Goto Track # CCW
  DCC_Accessory[12].Position2      =   30;                                     // Goto Track # CW
  DCC_Accessory[12].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[12].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[12].ReverseTrack1  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[12].ReverseTrack2  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[12].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[12].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[12].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[13].Address        =  413;                                     // DCC Address
  DCC_Accessory[13].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[13].Position1      =   31;                                     // Goto Track # CCW
  DCC_Accessory[13].Position2      =   13;                                     // Goto Track # CW
  DCC_Accessory[13].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[13].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[13].ReverseTrack1  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[13].ReverseTrack2  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[13].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[13].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[13].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[14].Address        =  414;                                     // DCC Address
  DCC_Accessory[14].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[14].Position1      =   32;                                     // Goto Track # CCW
  DCC_Accessory[14].Position2      =   14;                                     // Goto Track # CW
  DCC_Accessory[14].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[14].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[14].ReverseTrack1  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[14].ReverseTrack2  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[14].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[14].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[14].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[15].Address        =  415;                                     // DCC Address
  DCC_Accessory[15].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[15].Position1      =   33;                                     // Goto Track # CCW
  DCC_Accessory[15].Position2      =   15;                                     // Goto Track # CW
  DCC_Accessory[15].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[15].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[15].ReverseTrack1  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[15].ReverseTrack2  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[15].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[15].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[15].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[16].Address        =  416;                                     // DCC Address
  DCC_Accessory[16].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[16].Position1      =   34;                                     // Goto Track # CCW
  DCC_Accessory[16].Position2      =   16;                                     // Goto Track # CW
  DCC_Accessory[16].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[16].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[16].ReverseTrack1  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[16].ReverseTrack2  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[16].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[16].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[16].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[17].Address        =  417;                                     // DCC Address
  DCC_Accessory[17].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[17].Position1      =   35;                                     // Goto Track # CCW
  DCC_Accessory[17].Position2      =   17;                                     // Goto Track # CW
  DCC_Accessory[17].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[17].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[17].ReverseTrack1  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[17].ReverseTrack2  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[17].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[17].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[17].durationMilli  =  250;                                     // Pulse Time in ms

  DCC_Accessory[18].Address        =  418;                                     // DCC Address
  DCC_Accessory[18].Button         =    0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[18].Position1      =   36;                                     // Goto Track # CCW
  DCC_Accessory[18].Position2      =   18;                                     // Goto Track # CW
  DCC_Accessory[18].OutputPin1     =   14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[18].OutputPin2     =   15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[18].ReverseTrack1  =    1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[18].ReverseTrack2  =    0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[18].Finished       =    1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[18].Active         =    0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[18].durationMilli  =  250;                                     // Pulse Time in ms
} // END DCC_Accessory_ConfigureDecoderFunctions

//-----------------------------------------------------------------------------//
void DCC_Accessory_CheckStatus()
{
  for (uint8_t AccDec = 0; AccDec < DCC_Max_Accessories; AccDec++)             // Begin loop through DCC Accessory Decoders
  {
    DCC.loop();                                                                // Loop DCC Library
    if (DCC_Accessory[AccDec].Finished && DCC_Accessory[AccDec].Active)
    {
      DCC_Accessory[AccDec].Finished = 0;
      DCC_Accessory[AccDec].offMilli = millis() + DCC_Accessory[AccDec].durationMilli;
      switch (DCC_Accessory[AccDec].Address)
      {
        case (400):                                                            // DCC Address 400 0 = TURN 180, 1 = ????????
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button    : 0 = TURN 180
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin
            if (Turntable_CurrentTrack < 19)
            {
              Turntable_NewTrack = Turntable_CurrentTrack + (maxTrack / 2);
            } // END if
            else
            {
              Turntable_NewTrack = Turntable_CurrentTrack - (maxTrack / 2);
            } // END else
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_T180;                                    // Set New Action: Turn Motor (maxTrack / 2) Steps
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button  : 1 = ????????
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin
            if (Turntable_CurrentTrack < 19)
            {
              Turntable_NewTrack = Turntable_CurrentTrack + (maxTrack / 2);
            } // END if
            else
            {
              Turntable_NewTrack = Turntable_CurrentTrack - (maxTrack / 2);
            } // END else
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_T180;                                    // Set New Action: Turn Motor (maxTrack / 2) Steps
          } // END if
          break; // END case 226
  
        default:                                                               // DCC Address xxx to DCC_Max_Accessories
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button   : 0 = Goto Track Position1
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin1;                 // Set Arduino Output Pin - case default 0 = Red
            Turntable_NewTrack = DCC_Accessory[AccDec].Position1;              // Set New Turntable Track from DCC Address
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_TURN;                                    // Set New Action: DCC_TURN
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button : 1 = Goto Track Position2
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin - case default 1 = Green
            Turntable_NewTrack = DCC_Accessory[AccDec].Position2;              // Set New Turntable Track from DCC Address
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_TURN;                                    // Set New Action: DCC_TURN
          } // END if
          break; // END default
  
      } // END switch
      //              0123456789012345678901234                                // Sample text
      Serial.println();                                                        // Serial print text
	  Serial.print(F("(A) DCC: "));                                            // Serial print text
      Serial.print(DCC_Accessory[AccDec].Address);                             // Serial print value
      Serial.print(F(" - "));
      Serial.print(DCC_Accessory[AccDec].Button);                              // Serial print value
      Serial.print(F(" ("));                                                   // Serial print text
      Serial.print((DCC_Accessory[AccDec].Button) ? "G" : "R");                // 0 = Red, 1 = Green
      Serial.print(F(") --> "));                                               // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
      LCDPrintTrackStatus();                                                   // LCD print text
    } // END if
    if ((!DCC_Accessory[AccDec].Finished) && (millis() > DCC_Accessory[AccDec].offMilli))
    {
      DCC_Accessory[AccDec].Finished = 1;
      DCC_Accessory[AccDec].Active = 0;
    } // END if
  } // END for                                                                 // End loop through DCC Accessory Decoders
} // END DCC_Accessory_CheckStatus

//-----------------------------------------------------------------------------//
void Button_T180_CheckStatus()                                                 // Check Status Button T180
{
  Button_T180.loop();                                                          // Check debounce and update the state of the button
  Button_T180_CurrentState = Button_T180.getState();                           // HIGH = Released, LOW = Pressed
  if (Button_T180_LastState == HIGH && Button_T180_CurrentState == LOW)        // LOW = Pressed
  {
    Button_T180_IsPressed = true;
    Button_T180_IsPressing = true;
    Button_T180_ShortPressed = false;
    Button_T180_LongPressed = false;
    Button_T180_PressedTime = millis();
  } // END if
  else if (Button_T180_LastState == LOW && Button_T180_CurrentState == HIGH)   // HIGH = Released
  {
    Button_T180_IsPressing = false;
    Button_T180_ReleasedTime = millis();
    Button_T180_PressTime = Button_T180_ReleasedTime - Button_T180_PressedTime;
    if (Button_T180_PressTime < Button_T180_ShortPressTime)
    {
      Button_T180_ShortPressed = true;
    } // END if
    else if (Button_T180_LongPressed)
    {
      Button_T180_LongPressed = false;
    } // END else if
  } // END else if
  if (Button_T180_IsPressing && !Button_T180_LongPressed)
  {
    Button_T180_PressTime = millis() - Button_T180_PressedTime;
    if (Button_T180_PressTime >= Button_T180_LongPressTime)
    {
      Button_T180_LongPressed = true;
    } // END if
  } // END if
  if (Button_T180_ShortPressed)                                                // Button T180 Short Pressed : TURN 180
  {
    Button_T180_ShortPressed = false;
    DCC_Action_LED = 16;                                                       // Set Arduino Output Pin 16 = Yellow LED
    if (Turntable_CurrentTrack <= (maxTrack / 2))
    {
      Turntable_NewTrack = Turntable_CurrentTrack + (maxTrack / 2);
    } // END if
    else
    {
      Turntable_NewTrack = Turntable_CurrentTrack - (maxTrack / 2);
    } // END else
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_T180;                                            // Set New Action: Turn Motor (maxTrack / 2) Steps
    //              0123456789012345678901234                                  // Sample text
    Serial.println();                                                          // Serial print text
    Serial.print(F("BUT_T180 Short: "));                                       // Serial print text
    Serial.print(Button_T180_PressTime);                                       // Serial print value
    if (Button_T180_PressTime < 100)
    {
      Serial.print(F("   --> "));                                              // Serial print text
    } // END if
    else if (Button_T180_PressTime < 1000)
    {
      Serial.print(F("  --> "));                                               // Serial print text
    } // END else if
    else
    {
      Serial.print(F(" --> "));                                                // Serial print text
    } // END else
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  if (Button_T180_IsPressed && Button_T180_LongPressed)                        // Button T180 Long Pressed : Store current position as track 1
  {
    Button_T180_IsPressed = false;
    DCC_Action_LED = 16;                                                       // Set Arduino Output Pin 16 = Yellow LED
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_STORE;                                           // Set New Action: Store current position as track 1
    //              0123456789012345678901234                                  // Sample text
    Serial.println();                                                          // Serial print text
    Serial.print(F("BUT_T180 Long: "));                                        // Serial print text
    Serial.print(Button_T180_PressTime);                                       // Serial print value
    Serial.print(F("  --> "));                                                 // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  Button_T180_LastState = Button_T180_CurrentState;
} // END Button_T180_CheckStatus

//-----------------------------------------------------------------------------//
void Button_Right_CheckStatus()                                                // Check Status Button RIGHT
{
  Button_Right.loop();                                                         // Check debounce and update the state of the button
  Button_Right_CurrentState = Button_Right.getState();                         // HIGH = Released, LOW = Pressed
  if (Button_Right_LastState == HIGH && Button_Right_CurrentState == LOW)      // LOW = Pressed
  {
    Button_Right_IsPressed = true;
    Button_Right_IsPressing = true;
    Button_Right_ShortPressed = false;
    Button_Right_LongPressed = false;
    Button_Right_PressedTime = millis();
  } // END if
  else if (Button_Right_LastState == LOW && Button_Right_CurrentState == HIGH) // HIGH = Released
  {
    Button_Right_IsPressing = false;
    Button_Right_ReleasedTime = millis();
    Button_Right_PressTime = Button_Right_ReleasedTime - Button_Right_PressedTime;
    if (Button_Right_PressTime < Button_Right_ShortPressTime)
    {
      Button_Right_ShortPressed = true;
    } // END if
    else if (Button_Right_LongPressed)
    {
      Button_Right_LongPressed = false;
    } // END else if
  } // END else if
  if (Button_Right_IsPressing && !Button_Right_LongPressed)
  {
    Button_Right_PressTime = millis() - Button_Right_PressedTime;
    if (Button_Right_PressTime >= Button_Right_LongPressTime)
    {
      Button_Right_LongPressed = true;
    } // END if
  } // END if
  if (Button_Right_ShortPressed)                                               // Button RIGHT Short Pressed : TURN 1 CW
  {
    Button_Right_ShortPressed = false;
    DCC_Action_LED = 14;                                                       // Set Arduino Output Pin 14 = Red LED
    Turntable_NewTrack = Turntable_CurrentTrack + 1;
    if (Turntable_NewTrack > maxTrack)                                         // From Track 36 to Track 1
    {
      Turntable_NewTrack = 1;                                                  // Track (1)
    } // END if
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_T1CW;                                            // Set New Action: Turn Motor 1 Step ClockWise
    //              0123456789012345678901234                                  // Sample text
    Serial.println();                                                          // Serial print text
    Serial.print(F("BUT_RIGHT Short: "));                                      // Serial print text
    Serial.print(Button_Right_PressTime);                                      // Serial print value
    if (Button_Right_PressTime < 100)
    {
      Serial.print(F("  --> "));                                               // Serial print text
    } // END if
    else if (Button_Right_PressTime < 1000)
    {
      Serial.print(F(" --> "));                                                // Serial print text
    } // END if
    else
    {
      Serial.print(F("--> "));                                                 // Serial print text
    } // END else
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  if (Button_Right_IsPressed && Button_Right_LongPressed)                      // Button RIGHT Long Pressed : Reset Turntable Lock
  {
    Button_Right_IsPressed = false;
    DCC_Action_LED = 14;                                                       // Set Arduino Output Pin 14 = Red LED
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_RLOCK;                                           // Set New Action: Reset Turntable Lock
    //              0123456789012345678901234                                  // Sample text
    Serial.println();                                                          // Serial print text
    Serial.print(F("BUT_RIGHT Long: "));                                       // Serial print text
    Serial.print(Button_Right_PressTime);                                      // Serial print value
    Serial.print(F(" --> "));                                                  // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  Button_Right_LastState = Button_Right_CurrentState;
} // END Button_Right_CheckStatus

//-----------------------------------------------------------------------------//
void Button_Left_CheckStatus()                                                 // Check Status Button LEFT
{
  Button_Left.loop();                                                          // Check debounce and update the state of the button
  Button_Left_CurrentState = Button_Left.getState();                           // HIGH = Released, LOW = Pressed
  if (Button_Left_LastState == HIGH && Button_Left_CurrentState == LOW)        // LOW = Pressed
  {
    Button_Left_IsPressed = true;
    Button_Left_IsPressing = true;
    Button_Left_ShortPressed = false;
    Button_Left_LongPressed = false;
    Button_Left_PressedTime = millis();
  } // END if
  else if (Button_Left_LastState == LOW && Button_Left_CurrentState == HIGH)   // HIGH = Released
  {
    Button_Left_IsPressing = false;
    Button_Left_ReleasedTime = millis();
    Button_Left_PressTime = Button_Left_ReleasedTime - Button_Left_PressedTime;
    if (Button_Left_PressTime < Button_Left_ShortPressTime)
    {
      Button_Left_ShortPressed = true;
    } // END if
    else if (Button_Left_LongPressed)
    {
      Button_Left_LongPressed = false;
    } // END else if
  } // END else if
  if (Button_Left_IsPressing && !Button_Left_LongPressed)
  {
    Button_Left_PressTime = millis() - Button_Left_PressedTime;
    if (Button_Left_PressTime >= Button_Left_LongPressTime)
    {
      Button_Left_LongPressed = true;
    } // END if
  } // END if
  if (Button_Left_ShortPressed)                                                // Button LEFT Short Pressed : TURN 1 CCW
  {
    Button_Left_ShortPressed = false;
    DCC_Action_LED = 15;                                                       // Set Arduino Output Pin 15 = Green LED
    Turntable_NewTrack = Turntable_CurrentTrack - 1;
    if (Turntable_NewTrack == 0)                                               // From Track 1 to Track 36
    {
      Turntable_NewTrack = maxTrack;                                           // Track (maxTrack)
    } // END if
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_T1CCW;                                           // Set New Action: Turn Motor 1 Step ClockWise
    //              0123456789012345678901234                                  // Sample text
    Serial.println();                                                          // Serial print text
    Serial.print(F("BUT_LEFT Short: "));                                       // Serial print text
    Serial.print(Button_Left_PressTime);                                       // Serial print value
    if (Button_Left_PressTime < 100)
    {
      Serial.print(F("   --> "));                                              // Serial print text
    } // END if
    else if (Button_Left_PressTime < 1000)
    {
      Serial.print(F("  --> "));                                               // Serial print text
    } // END if
    else
    {
      Serial.print(F(" --> "));                                                // Serial print text
    } // END else
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  if (Button_Left_IsPressed && Button_Left_LongPressed)                        // Button LEFT Long Pressed : Set Turntable Lock
  {
    Button_Left_IsPressed = false;
    DCC_Action_LED = 15;                                                       // Set Arduino Output Pin 15 = Green LED
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_SLOCK;                                           // Set New Action: Set Turntable Lock
    //              0123456789012345678901234                                  // Sample text
    Serial.println();                                                          // Serial print text
    Serial.print(F("BUT_LEFT Long: "));                                        // Serial print text
    Serial.print(Button_Left_PressTime);                                       // Serial print value
    Serial.print(F("  --> "));                                                 // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  Button_Left_LastState = Button_Left_CurrentState;
} // END Button_Left_CheckStatus

//-----------------------------------------------------------------------------//
void Turntable_CheckSwitch()                                                   // From HIGH to LOW = Bridge in Position
{
  Turntable_Switch.loop();                                                     // Check debounce and update the state of the Turntable Switch
  if (Turntable_Switch.isPressed())                                            // Bridge in Position
  {
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = POS;                                                 // Set New Action: Bridge in Position
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(A) TT_CheckSwitch   --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
} // END Turntable_CheckSwitch

//-----------------------------------------------------------------------------//
void Turntable_StoreTrack()                                                    // Store Track Position in EEPROM
{
  digitalWrite(Turntable_StatusLED, HIGH);                                     // Set Arduino Onboard LED 17 = Blue LED = Bridge in Position ON
  digitalWrite(DCC_Action_LED, LOW);                                           // DCC Action LED OFF
  EEPROM.update(EE_Address, Turntable_CurrentTrack);                           // Store Turntable bridge position into EEPROM
  Serial.print(F("(A) EEPROM Track "));                                        // Serial print text
  if (Turntable_CurrentTrack < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  } // END if
  Serial.print(Turntable_CurrentTrack);                                        // Serial print value
  Serial.print(F("  --> "));                                                   // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END Turntable_StoreTrack

//-----------------------------------------------------------------------------//
void Turntable_CheckPos()                                                      // Check if Bridge on wanted track position
{
  if (Turntable_CurrentTrack == Turntable_NewTrack)                            // Bridge in Position = Stop Motor
  {
    switch (Turntable_OldAction)                                               // Check OldAction
    {
      case MCW:                                                                // Turntable turning ClockWise
        speedValue = 0;                                                        // Zero = Direction Stop - Old Action = MCW
        Turntable_SetMotorSpeed(speedValue);                                   // Motor Stop - Old Action = MCW
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = STOP;                                            // Set New Action: STOP
        //              0123456789012345678901234                              // Sample text
        Serial.print(F("(A) TT_CheckPos MCW  --> "));                          // Serial print text
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCW
        
      case MCCW:                                                               // Turntable turning Counter ClockWise
        speedValue = 0;                                                        // Zero = Direction Stop - Old Action = MCCW
        Turntable_SetMotorSpeed(speedValue);                                   // Motor Stop - Old Action = MCCW
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = STOP;                                            // Set New Action: STOP
        //              0123456789012345678901234                              // Sample text
        Serial.print(F("(A) TT_CheckPos MCCW --> "));                          // Serial print text
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCCW
        
      default:                                                                 // None of the above
        //              0123456789012345678901234                              // Sample text
        Serial.print(F("(A) TT_CheckPos "));                                   // Serial print text
        Serial.print(Turntable_OldAction);                                     // Serial print value
        Serial.print(F(" --> "));                                              // Serial print text
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END default

    } // END switch
  } // END if
  else                                                                         // Bridge NOT in Position = Don't Stop Motor
  {
    switch (Turntable_OldAction)                                               // Check OldAction
    {
      case MCW:                                                                // Turntable turning ClockWise
        digitalWrite(Turntable_StatusLED, LOW);                                // Set Arduino Onboard LED 17 = Blue LED = Bridge in Position OFF
        digitalWrite(DCC_Action_LED  , HIGH);                                  // Set DCC Action LED ON
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = MCW;                                             // Set New Action: MCW
        //              0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos MCW      --> "));                          // Serial print text
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCW
        
      case MCCW:                                                               // Turntable turning Counter ClockWise
        digitalWrite(Turntable_StatusLED, LOW);                                // Set Arduino Onboard LED 17 = Blue LED = Bridge in Position OFF
        digitalWrite(DCC_Action_LED  , HIGH);                                  // Set DCC Action LED ON
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = MCCW;                                            // Set New Action: MCCW
        //              0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos MCCW     --> "));                          // Serial print text
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCCW
        
      default:                                                                 // None of the above
        //              0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos "));                                       // Serial print text
        Serial.print(Turntable_OldAction);                                     // Serial print value
        Serial.print(F(" --> "));                                              // Serial print text
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END default
        
    } // END switch
  } // END else
} // END Turntable_CheckPos

//-----------------------------------------------------------------------------//
void DCC_Accessory_LED_OFF()                                                   // All LEDs OFF
{
  for (uint8_t AccDec = 0; AccDec < DCC_Max_Accessories; AccDec++)             // Begin loop through DCC Accessory Decoders
  {
    digitalWrite(DCC_Accessory[AccDec].OutputPin1, LOW);                       // LED OFF
    digitalWrite(DCC_Accessory[AccDec].OutputPin2, LOW);                       // LED OFF
  } // END for
} // END DCC_Accessory_LED_OFF

//-----------------------------------------------------------------------------//
void DCC_Action_LED_Startup()                                                  // All DCC Action LEDs ON and OFF
{
  for (int DCC_Action_LED = 14; DCC_Action_LED <= 17; DCC_Action_LED++)        // Loop DCC Action LED
  {                                                                            // Short LED test at startup
    digitalWrite(DCC_Action_LED, HIGH);                                        // DCC Action LED ON
    delay(200);                                                                // Simple delay for 200 msec
    digitalWrite(DCC_Action_LED, LOW);                                         // DCC Action LED OFF
  } // END for
} // END DCC_Action_LED_Startup

//-----------------------------------------------------------------------------//
void BridgeRelayInit()                                                         //  Initialize Reverse Tracks
{
  Serial.println(F("Initialize Reverse Tracks (0 = Normal, 1 = Reversed)"));
  for (uint8_t AccDec = 1; AccDec < DCC_Max_Accessories; AccDec++)             // DCC Accessory Decoder 0 is Turn 180 function. Start at #1
  {                                                                            // Configure Reverse Track (false = Normal, true = Reversed)
    Track1 = DCC_Accessory[AccDec].Position1;
    DCC_ReverseTrack[Track1] = DCC_Accessory[AccDec].ReverseTrack1;            // Set Reverse Track Status (false = Normal, true = Reversed)
    Serial.print(F("Address: "));                                              // Serial print text
    Serial.print(DCC_Accessory[AccDec].Address);                               // Serial print value
    Serial.print(F(",  Track "));                                              // Serial print text
    if (Track1 < 10)
    {
    Serial.print(F(" "));                                                      // Serial print text
    }
    Serial.print(Track1);                                                      // Serial print value
    Serial.print(F(": "));                                                     // Serial print text
    Serial.print(DCC_ReverseTrack[Track1]);                                    // Serial print value
    Serial.print(F(",  Track "));                                              // Serial print text
    if (Track1 <= 18)
    {
      DCC_ReverseTrack[Track1 + 18] = !DCC_Accessory[AccDec].ReverseTrack1;    // Set Reverse Track Status (false = Normal, true = Reversed)
      Serial.print(Track1 + 18);                                               // Serial print value
      Serial.print(F(": "));                                                   // Serial print text
      Serial.print(DCC_ReverseTrack[Track1 + 18]);                             // Serial print value
    } // END if
    else
    {
      DCC_ReverseTrack[Track1 - 18] = !DCC_Accessory[AccDec].ReverseTrack1;    // Set Reverse Track Status (false = Normal, true = Reversed)
      Serial.print(Track1 - 18);                                               // Serial print value
      Serial.print(F(": "));                                                   // Serial print text
      Serial.print(DCC_ReverseTrack[Track1 - 18]);                             // Serial print value
    } // END else
    Serial.println();                                                          // Serial print text
  } // END for
} // END BridgeRelayInit()

//-----------------------------------------------------------------------------//
void BridgeRelayCheck()                                                        // Check Bride Relay Status = ULN2803A
{
  if ((BridgeRelayActive) && (millis() > BridgeRelayMaxMillis))                // Check Bridge Relay Active Timer
  {
    BridgeRelayOff();                                                          // Disable Outputs to ULN2803A
  } // END if
  
} // END BridgeRelayCheck

//-----------------------------------------------------------------------------//
void BridgeRelaySet()                                                          // Set Bride Relay Status = ULN2803A
{
  if (BridgeRelayActive)                                                       // Check if Bridge Relay Set is active
  {
    switch (DCC_ReverseTrack[Turntable_CurrentTrack])
    {
      case (false):
        BridgeRelayOnNormal();                                                 // Enable Outputs to ULN2803A - Normal
        break; // END case false
      case (true):
        BridgeRelayOnReversed();                                               // Enable Outputs to ULN2803A - Reversed
        break; // END case true
    } // END switch
  } // END if
} // END BridgeRelaySet

//-----------------------------------------------------------------------------//
void BridgeRelayOff()                                                          // Disable Outputs to ULN2803A
{
  digitalWrite(BridgeRelayL1, LOW);                                            // Arduino Output Pin 7 OFF = ULN2803A Pin 1+2       = Bridge Relay L1-
  digitalWrite(BridgeRelayL2, LOW);                                            // Arduino Output Pin 8 OFF = ULN2803A Pin 3+4       = Bridge Relay L2-
  BridgeRelayMaxMillis = 0;                                                    // Reset Bridge Relay Active Timer
  BridgeRelayActive = false;                                                   // Set Bridge Relay Not Active
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("(A) Relay Free       --> "));                                // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END BridgeRelayOff

//-----------------------------------------------------------------------------//
void BridgeRelayOnNormal()                                                     // Enable Outputs to ULN2803A - Normal
{
  digitalWrite(BridgeRelayL1, HIGH);                                           // Arduino Output Pin 7 ON  = ULN2803A Pin 1+2       = Bridge Relay L1-
  digitalWrite(BridgeRelayL2, LOW);                                            // Arduino Output Pin 8 OFF = ULN2803A Pin 3+4       = Bridge Relay L2-
  BridgeRelayActive = true;                                                    // Set Bridge Relay Active
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("(A) Relay Normal     --> "));                                // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END BridgeRelayOnNormal

//-----------------------------------------------------------------------------//
void BridgeRelayOnReversed()                                                   // Enable Outputs to ULN2803A - Reversed
{
  digitalWrite(BridgeRelayL1, LOW);                                            // Arduino Output Pin 7 OFF = ULN2803A Pin 1+2       = Bridge Relay L1-
  digitalWrite(BridgeRelayL2, HIGH);                                           // Arduino Output Pin 8 ON  = ULN2803A Pin 3+4       = Bridge Relay L2-
  BridgeRelayActive = true;                                                    // Set Bridge Relay Active
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("(A) Relay Reversed   --> "));                                // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END BridgeRelayOnReversed

//-----------------------------------------------------------------------------//
void Turntable_SetMotorSpeed(int speed)                                        // Set Motor Speed
{
  DCC_Action_LED_Reset();                                                      // DCC Action LEDs OFF
  digitalWrite(DCC_Action_LED, HIGH);                                          // DCC Action LED ON
  switch (speed)
  {
    case (0):                                                                  // Motor Stop
      digitalWrite(Turntable_MotorM1, LOW);                                    // Output T.M1 L293D OFF
      digitalWrite(Turntable_MotorM2, LOW);                                    // Output T.M2 L293D OFF
      BridgeRelayMaxMillis = 0;                                                // Reset Bridge Relay Active Timer
      BridgeRelayActive = false;                                               // Deactivate Bridge Relay Set
      TurntableLockSetLock();                                                  // Enable Outputs to L293D - Lock Active
      break; // END case 0
    case (maxSpeed):                                                           // Motor Forward
      digitalWrite(Turntable_MotorM2, LOW);                                    // Output T.M2 L293D OFF
      analogWrite(Turntable_MotorM1, speed);                                   // PWM on T.M1 L293D
      BridgeRelayMaxMillis = millis() + BridgeRelayPulsTime;                   // Set Bridge Relay Active Timer
      BridgeRelayActive = true;                                                // Activate Bridge Relay Set
      break; // END case maxSpeed
    case (-maxSpeed):                                                          // Motor Reverse
      digitalWrite(Turntable_MotorM1, LOW);                                    // Output T.M1 L293D OFF
      analogWrite(Turntable_MotorM2, -speed);                                  // PWM on T.M2 L293D
      BridgeRelayMaxMillis = millis() + BridgeRelayPulsTime;                   // Set Bridge Relay Active Timer
      BridgeRelayActive = true;                                                // Activate Bridge Relay Set
      break; // END case -maxSpeed
  } // END switch
  LCDPrintTrackStatus();                                                       // LCD print text
} // END Turntable_SetMotorSpeed

//-----------------------------------------------------------------------------//
uint8_t GetZone(uint8_t Track )                                                // Detemine Zone from Track
{
  //            FromZone
  //           0  1  2  3
  //   T       |  |  |  |
  //   o  0 - {0, 1, 1, 0}
  //   Z  1 - {0, 0, 1, 0}
  //   o  2 - {0, 0, 0, 1}
  //   n  3 - {1, 0, 0, 0}
  //   e 
  uint8_t Zone;
  if      ( Track >=  0               && Track < (1*(maxTrack/4)) )            // Zone 0 = From  0 to  8 = Track  1 to 9
  {
    Zone = 0;
  } // END if
  else if ( Track >= (1*(maxTrack/4)) && Track < (2*(maxTrack/4)) )            // Zone 1 = From  9 to 17 = Track 10 to 18
  {
    Zone = 1;
  } // END else if
  else if ( Track >= (2*(maxTrack/4)) && Track < (3*(maxTrack/4)) )            // Zone 2 = From 18 to 26 = Track 19 to 27
  {
    Zone = 2;
  } // END else if
  else                                                                         // Zone 3 = From 27 to 35 = Track 28 to 36
  {
    Zone = 3;
  } // END else
  return Zone;
} // END GetZone

//-----------------------------------------------------------------------------//
void SetDirection()                                                            // Set Direction based Action or Zones
{
  uint8_t FromZone, ToZone;
  FromZone = GetZone( Turntable_CurrentTrack - 1 );                          // Detemine Zone from Current Track
  ToZone   = GetZone( Turntable_NewTrack - 1);                               // Detemine Zone from New Track
  if (Turntable_NewTrack == Turntable_CurrentTrack)                            // Check Current Track
  {
    speedValue = 0;                                                            // Zero = Direction Stop - Stop
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;                                                // Set New Action: STOP
  } // END if
  else                                                                         // Old Action unknown (probably STOP)
  {
    if ( ToZone != FromZone )                                                  // Only when FromZone not the same as ToZone
    {
      if ( Directions[ToZone][FromZone] )                                      // Directions matrix contains 1 = CCW
      {
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = TCCW;                                            // Set New Action: Turn Motor M1 Counter ClockWise
        speedValue = -maxSpeed;                                                // Negative = Direction Counter ClockWise
      } // END if
      else
      {
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = TCW;                                             // Set New Action: Turn Motor M1 ClockWise
        speedValue = maxSpeed;                                                 // Positive = Direction ClockWise
      } // END else
    } // END if
    else                                                                       // FromZone same as ToZone
    {
      if ( Turntable_NewTrack < Turntable_CurrentTrack )
      {
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = TCCW;                                            // Set New Action: Turn Motor M1 Counter ClockWise
        speedValue = -maxSpeed;                                                // Negative = Direction Counter ClockWise
      } // END if
      else
      {
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = TCW;                                             // Set New Action: Turn Motor M1 ClockWise
        speedValue = maxSpeed;                                                 // Positive = Direction ClockWise
      } // END else
    } // END else
  } // END else
  Serial.print(F("(A) From Zone "));                                           // Serial print text
  Serial.print(FromZone);                                                      // Serial print value
  Serial.print(F(" to "));                                                     // Serial print text
  Serial.print(ToZone);                                                        // Serial print value
  Serial.print(F(" --> "));                                                    // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END SetDirection

//-----------------------------------------------------------------------------//
void DCC_Action_LED_Reset()                                                    // DCC Action LEDs OFF
{
  for (int ledpin = 14; ledpin <= 17; ledpin++)                                // Loop ledpin
  {
    digitalWrite(ledpin, LOW);                                                 // Switch LED OFF
  } // END for
}

//-----------------------------------------------------------------------------//
void PrintStatus()                                                             // Print Actions and Track Numbers
{
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("Old: "));                                                    // Serial print text
  Serial.print(Turntable_States[Turntable_OldAction]);                         // Serial print action
  Serial.print(F(", New: "));                                                  // Serial print text
  Serial.print(Turntable_States[Turntable_NewAction]);                         // Serial print action
  Serial.print(F(", Track: "));                                                // Serial print text
  if (Turntable_CurrentTrack < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  } // END if
  Serial.print(Turntable_CurrentTrack);                                        // Serial print value
  Serial.print(F(" to "));                                                     // Serial print text
  if (Turntable_NewTrack < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  } // END if
  Serial.print(Turntable_NewTrack);                                            // Serial print value
  Serial.print(F(", LED: "));                                                  // Serial print text
  if (DCC_Action_LED < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  } // END if
  Serial.print(DCC_Action_LED);                                                // Serial print value
  Serial.print(F(", Speed: "));                                                // Serial print text
  if (speedValue == 0)
  {
    Serial.print(F("   "));                                                    // Serial print text
  } // END if
  if (speedValue > 0)
  {
    Serial.print(F(" "));                                                      // Serial print text
  } // END if
  Serial.print(speedValue);                                                    // Serial print value
  Serial.print(F(", Relay: "));                                                // Serial print text
  Serial.print(BridgeRelayActive);                                             // Serial print value
  Serial.print(F(", Lock: "));                                                 // Serial print text
  Serial.print(TurntableLockActive);                                           // Serial print value
  Serial.print(F(", Wait: "));                                                 // Serial print text
  Serial.print(TurntableWaitActive);                                           // Serial print value
  Serial.print(F(", Watchdog: "));                                             // Serial print text
  if (WatchdogCounter < 10)
  {
    Serial.print(F("  "));                                                     // Serial print text
  } // END if
  else if (WatchdogCounter < 100)
  {
    Serial.print(F(" "));                                                      // Serial print text
  } // END else if
  Serial.print(WatchdogCounter);                                               // Serial print value
  Serial.println();                                                            // Serial print text
  WatchdogCounter = 0;
} // END PrintStatus

//-----------------------------------------------------------------------------//
void LCDPrintTrackStatus()
{
  //           01234567890123456789                                            // Sample text
//lcd.print(F("Track: .. to .. #..."));                                        // LCD print text
  if (Turntable_CurrentTrack < 10)
  {
    lcd.setCursor(7, 3);                                                       // Set cursor to first line and 8th character
    lcd.print(F(" "));                                                         // LCD print text
  //lcd.setCursor(8, 3);                                                       // Set cursor to first line and 9th character
  } // END if
  else
  {
    lcd.setCursor(7, 3);                                                       // Set cursor to first line and 8th character
  } // END else
  lcd.print(Turntable_CurrentTrack);                                           // LCD print value
  if (Turntable_NewTrack < 10)
  {
    lcd.setCursor(13, 3);                                                      // Set cursor to first line and 14th character
    lcd.print(F(" "));                                                         // LCD print text
  //lcd.setCursor(14, 3);                                                      // Set cursor to first line and 15th character
  } // END if
  else
  {
    lcd.setCursor(13, 3);                                                      // Set cursor to first line and 14th character
  } // END else
  lcd.print(Turntable_NewTrack);                                               // LCD print value
  lcd.setCursor(16, 3);                                                        // Set cursor to first line and 17th character
  if (speedValue == 0)
  {
    lcd.print(F("-  "));                                                       // LCD print text
  } // END if
  if (speedValue < 0)
  {
    lcd.print(F("<"));                                                         // LCD print text
  } // END if
  if (speedValue > 0)
  {
    lcd.print(F(">"));                                                         // LCD print text
  } // END if
  lcd.setCursor(17, 3);                                                        // Set cursor to first line and 18th character
  if (abs(speedValue) < 100)
  {
    lcd.setCursor(18, 3);                                                      // Set cursor to first line and 19th character
  } // END if
  if (abs(speedValue) < 10)
  {
    lcd.setCursor(19, 3);                                                      // Set cursor to first line and 20th character
  } // END if
  lcd.print(abs(speedValue));                                                  // LCD print value
} // END LCDPrintTrackStatus

//-----------------------------------------------------------------------------//
void LCDPrintTrackText()                                                       // LCD print text
{
  lcd.setCursor(0, 3);                                                         // Set cursor to fourth line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("Track:    to        "));                                        // LCD print text
} // END LCDPrintTrackText

//-----------------------------------------------------------------------------//
void TurntableWaitCheck()                                                      // Check Turntable Wait Before Turn Status
{
  if (TurntableWaitActive && (millis() > TurntableWaitMillis))                 // Check Turntable Wait Before Turn Active Timer
  {
    TurntableWaitActive = false;                                               // Deactivate Turntable Wait Before Turn
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(A) Wait Ready       --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
} // END TurntableWaitCheck

//-----------------------------------------------------------------------------//
void TurntableLockCheck()                                                      // Check Turntable Lock Status = L293D
{
  if (TurntableLockActive && (millis() > TurntableLockMillis))                 // Check Turntable Lock Active Timer
  {
    TurntableLockOff();                                                        // Disable Outputs to L293D
  } // END if
} // END TurntableLockCheck

//-----------------------------------------------------------------------------//
void TurntableLockOff()                                                        // Disable Outputs to L293D
{
  digitalWrite(Turntable_LockL1, LOW);                                         // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  digitalWrite(Turntable_LockL2, LOW);                                         // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  TurntableLockMillis = 0;                                                     // Reset Turntable Lock timer
  TurntableLockActive = false;                                                 // Set Turntable Lock Not Active
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("(A) Lock Off         --> "));                                // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END TurntableLockOff

//-----------------------------------------------------------------------------//
void TurntableLockResetLock()                                                  // Enable Outputs to L293D - Lock Free
{
  digitalWrite(Turntable_LockL1, HIGH);                                        // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  digitalWrite(Turntable_LockL2, LOW);                                         // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  TurntableLockMillis = millis() + TurntableLockInterval;                      // Set Turntable Lock Active Timer
  TurntableLockActive = true;                                                  // Set Turntable Lock Active
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("(A) Lock Reset       --> "));                                // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END TurntableLockResetLock

//-----------------------------------------------------------------------------//
void TurntableLockSetLock()                                                    // Enable Outputs to L293D - Lock Active
{
  digitalWrite(Turntable_LockL1, LOW);                                         // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  digitalWrite(Turntable_LockL2, HIGH);                                        // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  TurntableLockMillis = millis() + TurntableLockInterval;                      // Set Turntable Lock Active Timer
  TurntableLockActive = true;                                                  // Set Turntable Lock Active
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("(A) Lock Set         --> "));                                // Serial print text
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END TurntableLockSetLock

//=============================================================================//
void loop()                                                                    // Arduino Main Program
{
  if (millis() - WatchdogMillis >= WatchdogInterval)                           // When Watchdog Interval is reached
  {
    WatchdogMillis = millis();                                                 // Set Watchdog timer
    WatchdogState = !WatchdogState;                                            // Change Watchdog State
    WatchdogCounter++;                                                         // Increase Watchdog counter
    digitalWrite(WatchdogLED, WatchdogState);                                  // Change Watchdog LED
  } // END if
  DCC_Accessory_CheckStatus();                                                 // Check DCC Accessory Status
  Button_T180_CheckStatus();                                                   // Check Status Button T180
  Button_Right_CheckStatus();                                                  // Check Status Button RIGHT
  Button_Left_CheckStatus();                                                   // Check Status Button LEFT
  Turntable_CheckSwitch();                                                     // Check Kato Turntable Pin 1
  BridgeRelayCheck();                                                          // Check Bride Relay Status = ULN2803A
  TurntableLockCheck();                                                        // Check Turntable Lock Status = L293D
  //===========================================================================//
  if (Turntable_NewAction == BUT_T180)                                         // Button T180  (Shortpress) = Turn 180
  {
    if (Turntable_OldAction != BUT_T180)                                       // Check Old Action
    {
      TurntableLockResetLock();                                                // Enable Outputs to L293D - Lock Free
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
    }
    if (!TurntableLockActive && (millis() > TurntableLockMillis))              // Check Turntable Lock Active Timer
    {
      if ((Turntable_CurrentTrack >= 1) && (Turntable_CurrentTrack <= 18))
      {
        speedValue = maxSpeed;                                                 // Positive = Direction ClockWise - BUT_T180
        Turntable_SetMotorSpeed(speedValue);                                   // Motor Forward - BUT_T180
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = MCW;                                             // Set New Action: Move Motor ClockWise
      } // END if
      else
      {
        speedValue = -maxSpeed;                                                // Negative = Direction Counter ClockWise - BUT_T180
        Turntable_SetMotorSpeed(speedValue);                                   // Motor Backward - BUT_T180
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = MCCW;                                            // Set New Action: Move Motor Counter ClockWise
      } // END else
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) BUT_T180         --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
	}
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction != BUT_STORE) && (Turntable_NewAction == BUT_STORE))// Button T180  (Longpress)  = Store current position as track 1
  {
    Turntable_CurrentTrack = 1;                                                // Bridge in Home Position
    Turntable_NewTrack = 1;                                                    // Bridge in Home Position
    DCC_Action_LED_Startup();                                                  // All DCC Action LEDs ON and OFF
    Turntable_StoreTrack();                                                    // Store Track Position in EEPROM
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;                                                // Set New Action: STOP
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) BUT_STORE        --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == BUT_T1CW)                                         // Button Right (Shortpress) = Turn 1 Step ClockWise
  {
    if (Turntable_OldAction != BUT_T1CW)                                       // Check Old Action
    {
      TurntableLockResetLock();                                                // Enable Outputs to L293D - Lock Free
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
    }
    if (!TurntableLockActive && (millis() > TurntableLockMillis))              // Check Turntable Lock Active Timer
    {
      speedValue = maxSpeed;                                                   // Positive = Direction ClockWise - BUT_T1CW
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Forward - BUT_T1CW
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCW;                                               // Set New Action: Move Motor ClockWise
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) BUT_T1CW         --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
	}
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == BUT_T1CCW)                                        // Button Left  (Shortpress) = Turn 1 Step Counter ClockWise
  {
    if (Turntable_OldAction != BUT_T1CCW)                                      // Check Old Action
    {
      TurntableLockResetLock();                                                // Enable Outputs to L293D - Lock Free
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
    }
    if (!TurntableLockActive && (millis() > TurntableLockMillis))              // Check Turntable Lock Active Timer
    {
      speedValue = -maxSpeed;                                                  // Negative = Direction Counter ClockWise - BUT_T1CCW
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Reverse - BUT_T1CCW
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCCW;                                              // Set New Action: Move Motor Counter ClockWise
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) BUT_T1CCW        --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
	}
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == BUT_RLOCK)                                        // Button Right (Longpress)  = Reset Turntable Lock
  {
    TurntableLockResetLock();                                                  // Enable Outputs to L293D - Lock Free
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;                                                // Set New Action: STOP
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) BUT_RLOCK        --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == BUT_SLOCK)                                        // Button Left  (Longpress)  = Set Turntable Lock
  {
    TurntableLockSetLock();                                                    // Enable Outputs to L293D - Lock Active
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;                                                // Set New Action: STOP
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) BUT_SLOCK        --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction != DCC_T180) && (Turntable_NewAction == DCC_T180))  // DCC Command Turn 180
  {
    if ((Turntable_CurrentTrack >= 1) && (Turntable_CurrentTrack <= 18))
    {
      speedValue = maxSpeed;                                                   // Positive = Direction ClockWise - DCC_T180
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Forward - DCC_T180
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCW;                                               // Set New Action: Move Motor ClockWise
    } // END if
    else
    {
      speedValue = -maxSpeed;                                                  // Negative = Direction Counter ClockWise - DCC_T180
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Backward - DCC_T180
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCCW;                                              // Set New Action: Move Motor Counter ClockWise
    } // END else
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) DCC_T180         --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction != DCC_TURN) && (Turntable_NewAction == DCC_TURN))  // DCC Command Turn - Button 401..436
  {
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = UNLOCKING;                                           // Set New Action: UNLOCK
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) DCC_TURN         --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == UNLOCKING)                                        // Unlocking Turntable Lock
  {
    if (Turntable_OldAction != UNLOCKING)                                      // Check Old Action
    {
      TurntableLockResetLock();                                                // Enable Outputs to L293D - Lock Free
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
    }
    if (!TurntableLockActive)
    {
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = UNLOCKED;                                          // Set New Action: UNLOCKED
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) UNLOCKING        --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
    } // END if
    if (TurntableLockActive && (millis() > TurntableLockMillis))               // Check Turntable Lock Active Timer
    {
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = UNLOCKED;                                          // Set New Action: UNLOCKED
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) UNLOCKING        --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
    } // END if
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == UNLOCKED)                                         // Turntable Lock Unlocked
  {
    TurntableLockOff();                                                        // Disable Outputs to L293D
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = TURN_WAIT;                                           // Set New Action: TURN_WAIT
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) UNLOCKED         --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == TURN_WAIT)                                        // Turntable Wait Before Turn Status
  {
    if (Turntable_OldAction != TURN_WAIT)                                      // Check Old Action
    {
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      TurntableWaitMillis = millis() + TurntableWaitInterval;                  // Set Turntable Wait Before Turn Active Timer
      TurntableWaitActive = true;                                              // Set Turntable Wait Before Turn Active
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) Waiting          --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
    }
    if (TurntableWaitActive && (millis() > TurntableWaitMillis))               // Check Turntable Wait Before Turn Active Timer
    {
      TurntableWaitActive = false;                                             // Deactivate Turntable Wait Before Turn
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = SETDIR;                                            // Set New Action: SETDIR
      //              0123456789012345678901234                                // Sample text
      Serial.print(F("(L) TURN_WAIT        --> "));                            // Serial print text
      PrintStatus();                                                           // Print Actions and Track Numbers
    } // END if
  } // END if
  //===========================================================================//
  if (Turntable_NewAction == SETDIR)                                           // Turntable Wait Before Turn Status
  {
    SetDirection();                                                            // Set Direction based Action or Zones
    // Turntable_OldAction                                                     // Old Action based on SETDIR
    // Turntable_NewAction                                                     // New Action based on SETDIR
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) SETDIR           --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction != TCW) && (Turntable_NewAction == TCW))            // Turn ClockWise
  {
    speedValue = maxSpeed;                                                     // Positive = Direction ClockWise - TCW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Forward - TCW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCW;                                                 // Set New Action: Move Motor ClockWise
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) Check TCW        --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction != TCCW) && (Turntable_NewAction == TCCW))          // Turn Counter ClockWise
  {
    speedValue = -maxSpeed;                                                    // Negative = Direction Counter ClockWise - TCCW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Reverse - TCCW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCCW;                                                // Set New Action: Move Motor Counter ClockWise
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) Check TCCW       --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction == MCW) && (Turntable_NewAction == POS))            //  Turntable in Position after Move ClockWise
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack + 1;
    if (Turntable_CurrentTrack > maxTrack)                                     // From Track 36 to Track 1
    {
      Turntable_CurrentTrack = 1;                                              // Track (1)
    } // END if
    Turntable_CheckPos();                                                      // Check if Bridge on wanted track position
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) T+1 - Check MCW  --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction == MCCW) && (Turntable_NewAction == POS))           //  Turntable in Position after Move Counter ClockWise
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack - 1;
    if (Turntable_CurrentTrack == 0)                                           // From Track 1 to Track 36
    {
      Turntable_CurrentTrack = maxTrack;                                       // Track (maxTrack)
    } // END if
    Turntable_CheckPos();                                                      // Check if Bridge in Position
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) T-1 - Check MCCW --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if
  //===========================================================================//
  if ((Turntable_OldAction == POS) && (Turntable_NewAction == STOP))           // STOP after Bridge in Position
  {
    Turntable_StoreTrack();                                                    // Store Track Position in EEPROM
    BridgeRelayActive = true;                                                  // Activate Bridge Relay Set
    BridgeRelaySet();                                                          // Set Bride Relay Status = ULN2803A
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;                                                // Set New Action: STOP
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(L) STOP             --> "));                              // Serial print text
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
} // END loop
//-----------------------------------------------------------------------------//
