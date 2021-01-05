//-----------------------------------------------------------------------------// DCCNext-Controlled-Kato-Turntable_v1.48
#include <EEPROM.h>                                                            // Standard Arduino EEPROM library
#include <DCC_Decoder.h>                                                       // Use Manage Libraries to add: NmraDcc -- https://github.com/MynaBay/DCC_Decoder
#include <ezButton.h>                                                          // Use Manage Libraries to add: ezButton -- https://github.com/ArduinoGetStarted/button
#include <LiquidCrystal_I2C.h>                                                 // Use Manage Libraries to add: LiquidCrystal I2C -- https://github.com/johnrickman/LiquidCrystal_I2C
#define maxSpeed                        100                                    // Speed between -255 = Reversed to 255 = Forward (-5 to +5 VDC)
#define maxTrack                         36                                    // Total Number of Turntable Tracks
#define kDCC_INTERRUPT                    0                                    // DCC Interrupt 0
#define DCC_Address_Offset                1                                    // Default = 1, for Multimaus = 4
#define DCC_Max_Accessories              13                                    // Total Number of DCC Accessory Decoder Addresses = 225-237
#define DCC_Interrupt                     2                                    // Arduino Input  Pin  2 = DCC signal = Interrupt 0
#define Turntable_MotorM1                 5                                    // Arduino Output Pin  5 = Turntable Motor     = Turntable Cable Pin 3
#define Turntable_MotorM2                 6                                    // Arduino Output Pin  6 = Turntable Motor     = Turntable Cable Pin 4
#define BridgeRelayL1                     7                                    // Arduino Output Pin  7 = ULN2803A Pin 1+2    = Bridge Relay L1-
#define BridgeRelayL2                     8                                    // Arduino Output Pin  8 = ULN2803A Pin 3+4    = Bridge Relay L2-
#define Turntable_LockL2                  9                                    // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
#define Turntable_LockL1                 10                                    // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
#define WatchdogLED                      13                                    // Arduino Output Pin 13 = DCCNext Red LED     = Watchdog Blink
#define RedLED                           14                                    // Arduino Output Pin 14 = Red LED             = Function Red
#define GreenLED                         15                                    // Arduino Output Pin 15 = Green LED           = Function Green
#define YellowLED                        16                                    // Arduino Output Pin 16 = Yellow LED          = Turn 180
#define Turntable_StatusLED              17                                    // Arduino Output Pin 17 = Blue LED            = Bridge in Position
                                                                               // Arduino VCC Pin       = Bridge Relay L1+
                                                                               // Arduino VCC Pin       = Bridge Relay L2+
                                                                               // Arduino Ground Pin GND                      = Turntable Cable Pin 2
ezButton Turntable_Switch(3);                                                  // Arduino Input  Pin  3 = Turntable Trigger   = Turntable Cable Pin 1
ezButton Button_T180(4);                                                       // Arduino Input  Pin  4 = Button Turn 180     = Turn 180
ezButton Button_Right(11);                                                     // Arduino Input  Pin 11 = Button Turn Right   = Turn 1 Step ClockWise
ezButton Button_Left(12);                                                      // Arduino Input  Pin 12 = Button Turn Left    = Turn 1 Step Counter ClockWise
uint32_t Button_T180_PressedTime     =     0;                                  // Button Turn 180 pressed time in ms
uint32_t Button_T180_ReleasedTime    =     0;                                  // Button Turn 180 released time in ms
uint32_t Button_T180_PressTime       =     0;                                  // Button Turn 180 press time in ms
uint32_t Button_T180_ShortPressTime  =  1000;                                  // Button Turn 180 short press time in ms
uint32_t Button_T180_LongPressTime   =  3000;                                  // Button Turn 180 long press time in ms
boolean  Button_T180_CurrentState    =  HIGH;                                  // Button Turn 180 current state (Released = HIGH)
boolean  Button_T180_LastState       =  HIGH;                                  // Button Turn 180 last state (Released = HIGH)
boolean  Button_T180_IsPressed       = false;                                  // Button Turn 180 pressed
boolean  Button_T180_IsPressing      = false;                                  // Button Turn 180 pressing
boolean  Button_T180_ShortPressed    = false;                                  // Button Turn 180 short pressed
boolean  Button_T180_LongPressed     = false;                                  // Button Turn 180 long pressed
uint8_t  DCC_Action_LED             =     0;                                   // Pin Number will change by DCC command (pin 14 = Red , 15 = Green, 16 = Blue)
uint8_t  Turntable_CurrentTrack     =     0;                                   // Turntable Current Track
uint8_t  Turntable_NewTrack         =     0;                                   // Turntable New Track
uint8_t  EE_Address                 =     0;                                   // EEPROM Address Turntable Bridge Position
int      speedValue                 =     0;                                   // Turntable Motor Speed = 0 - 255
boolean  DCC_ReverseTrack[37];                                                 // Status will change by DCC command (false = Normal, true = Reversed)
// Note: Size of DCC_ReverseTrack must be maxTrack+1 !!                        // --> [0..maxTrack] = maxTrack+1 records !!                                                                               // Note: Size of DCC_ReverseTrack must be the same as maxTrack !!
const uint32_t WatchdogInterval     =   250;                                   // Watchdog blink interval in ms
uint32_t WatchdogMillis             =     0;                                   // Last time Watchdog LED was updated
boolean  WatchdogState              =   LOW;                                   // Watchdog LED state
boolean  BridgeRelayActive          = false;                                   // Bridge Relay Not Active
uint32_t BridgeRelayMaxMillis       =     0;                                   // Bridge Relay Active Timer
uint32_t BridgeRelayPulsTime        =   250;                                   // Bridge Relay Pulse Time in ms
uint32_t TurntableLockMillis        =     0;                                   // Turntable Lock Active Timer
uint32_t TurntableLockInterval      =   500;                                   // Turntable Lock Pulse Time in ms
boolean  TurntableLockActive        = false;                                   // Turntable Lock Not Active
uint8_t  Track1                     =     0;                                   // Temp variables
uint8_t  Track2                     =     0;                                   // Temp variables

const char* Turntable_States[] =                                               // Possible Turntable States
{//012345678
  "STOP     ",                                                                 // Stop Turning
  "POS      ",                                                                 // Bridge in Position
  "TCW      ",                                                                 // Turn ClockWise
  "TCCW     ",                                                                 // Turn Counter ClockWise
  "MCW      ",                                                                 // Motor ClockWise
  "MCCW     ",                                                                 // Motor Counter ClockWise
  "DCC_END  ",                                                                 // DCC Command END                           - Button 225: 0 = OFF (Red)
  "DCC_INPUT",                                                                 // DCC Command INPUT                         - Button 225: 1 = ON  (Green)
  "DCC_CLEAR",                                                                 // DCC Command CLEAR                         - Button 226: 0 = OFF (Red)
  "DCC_T180 ",                                                                 // DCC Command Turn 180                      - Button 226: 1 = ON  (Green)
  "DCC_T1CW ",                                                                 // DCC Command Turn 1 Step ClockWise         - Button 227: 0 = OFF (Red)
  "DCC_T1CCW",                                                                 // DCC Command Turn 1 Step Counter ClockWise - Button 227: 1 = ON  (Green)
  "DCC_DCW  ",                                                                 // DCC Command Direction ClockWise           - Button 228: 0 = OFF (Red)
  "DCC_DCCW ",                                                                 // DCC Command Direction Counter ClockWise   - Button 228: 1 = ON  (Green)
  "BUT_T180 ",                                                                 // Button T180  = Turn 180
  "BUT_T1CW ",                                                                 // Button Right = Turn 1 Step ClockWise
  "BUT_T1CCW",                                                                 // Button Left  = Turn 1 Step Counter ClockWise
  "BUT_STORE"                                                                  // Button T180  = Store current position as track 1
}; // END const

enum Turntable_NewActions:uint8_t                                              // Possible Turntable Actions
{
  STOP       ,                                                                 // Stop Turning
  POS        ,                                                                 // Bridge in Position
  TCW        ,                                                                 // Turn ClockWise
  TCCW       ,                                                                 // Turn Counter ClockWise
  MCW        ,                                                                 // Motor ClockWise
  MCCW       ,                                                                 // Motor Counter ClockWise
  DCC_END    ,                                                                 // DCC Command END
  DCC_INPUT  ,                                                                 // DCC Command INPUT
  DCC_CLEAR  ,                                                                 // DCC Command CLEAR
  DCC_T180   ,                                                                 // DCC Command Turn 180
  DCC_T1CW   ,                                                                 // DCC Command Turn 1 Step ClockWise
  DCC_T1CCW  ,                                                                 // DCC Command Turn 1 Step Counter ClockWise
  DCC_DCW    ,                                                                 // DCC Command Direction ClockWise
  DCC_DCCW   ,                                                                 // DCC Command Direction Counter ClockWise
  BUT_T180   ,                                                                 // Button T180  = Turn 180
  BUT_T1CW   ,                                                                 // Button Right = Turn 1 Step ClockWise
  BUT_T1CCW  ,                                                                 // Button Left  = Turn 1 Step Counter ClockWise
  BUT_STORE                                                                    // Button T180  = Store current position as track 1
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
  Serial.println(F("DCCNext-Controlled-Kato-Turntable_v1.48 -- (c)JMRRvS 2021-01-04"));
                                                                               // Serial print loaded sketch
  lcd.init();                                                                  // Initialise LCD
  lcd.backlight();                                                             // Switch backlight ON
  lcd.setCursor(0, 0);                                                         // Set cursor to first line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("DCCNext Controlled  "));                                        // LCD print text
  lcd.setCursor(0, 1);                                                         // Set cursor to second line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("Kato Turntable v1.48"));                                        // LCD print text
  lcd.setCursor(0, 2);                                                         // Set cursor to third line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("--------------------"));                                        // LCD print text
  lcd.setCursor(0, 3);                                                         // Set cursor to fourth line and left corner
  Turntable_Switch.setDebounceTime(10);                                        // Set Debounce Time to 10 milliseconds
  Button_T180.setDebounceTime(50);                                             // Set Debounce Time to 50 milliseconds
  Button_Right.setDebounceTime(50);                                            // Set Debounce Time to 50 milliseconds
  Button_Left.setDebounceTime(50);                                             // Set Debounce Time to 50 milliseconds
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
  //                  0123456789012345678901234                                // Sample text
    Serial.println(F("EEPROM status unknown.   "));                            // Serial print Function
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
  //              0123456789012345678901234                                    // Sample text
  Serial.print(F("Startup_Status       --> "));                                // Serial print Function
  PrintStatus();                                                               // Print Actions and Track Numbers
  LCDPrintTrackText();                                                         // LCD print text
  LCDPrintTrackStatus();                                                       // LCD print text
} // END setup


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


void DCC_Accessory_ConfigureDecoderFunctions()
{
  DCC_Accessory[0].Address        =   225;                                     // DCC Address 225 0 = END, 1 = INPUT
  DCC_Accessory[0].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[0].Position1      =     0;                                     // Turntable Position1 - not used in this function
  DCC_Accessory[0].Position2      =     0;                                     // Turntable Position2 - not used in this function
  DCC_Accessory[0].OutputPin1     =     0;                                     // Arduino Output Pin xx = LED xx - not used in this function
  DCC_Accessory[0].OutputPin2     =     0;                                     // Arduino Output Pin xx = LED xx - not used in this function
  DCC_Accessory[0].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[0].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[0].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[0].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[0].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[1].Address        =   226;                                     // DCC Address 226 0 = CLEAR, 1 = TURN 180
  DCC_Accessory[1].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[1].Position1      =     0;                                     // Turntable Position1 - not used in this function
  DCC_Accessory[1].Position2      =     0;                                     // Turntable Position2 - not used in this function
  DCC_Accessory[1].OutputPin1     =     0;                                     // Arduino Output Pin xx = LED xx - not used in this function
  DCC_Accessory[1].OutputPin2     =    16;                                     // Arduino Output Pin 16 = Yellow LED
  DCC_Accessory[1].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[1].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[1].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[1].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[1].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[2].Address        =   227;                                     // DCC Address 227 0 = 1 STEP CW, 1 = 1 STEP CCW
  DCC_Accessory[2].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[2].Position1      =     0;                                     // Turntable Position1 - not used in this function
  DCC_Accessory[2].Position2      =     0;                                     // Turntable Position2 - not used in this function
  DCC_Accessory[2].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[2].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[2].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[2].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[2].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[2].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[2].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[3].Address        =   228;                                     // DCC Address 228 0 = Direction CW, 1 = Direction CCW
  DCC_Accessory[3].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[3].Position1      =     0;                                     // Turntable Position1 - not used in this function
  DCC_Accessory[3].Position2      =     0;                                     // Turntable Position1 - not used in this function
  DCC_Accessory[3].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[3].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[3].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[3].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[3].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[3].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[3].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[4].Address        =   229;                                     // DCC Address 229 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[4].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[4].Position1      =     1;                                     // Turntable Track 1
  DCC_Accessory[4].Position2      =     2;                                     // Turntable Track 2
  DCC_Accessory[4].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[4].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[4].ReverseTrack1  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[4].ReverseTrack2  =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[4].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[4].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[4].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[5].Address        =   230;                                     // DCC Address 230 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[5].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[5].Position1      =     3;                                     // Turntable Track 3
  DCC_Accessory[5].Position2      =     4;                                     // Turntable Track 4
  DCC_Accessory[5].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[5].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[5].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[5].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[5].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[5].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[5].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[6].Address        =   231;                                     // DCC Address 231 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[6].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[6].Position1      =     5;                                     // Turntable Track 5
  DCC_Accessory[6].Position2      =     6;                                     // Turntable Track 6
  DCC_Accessory[6].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[6].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[6].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[6].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[6].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[6].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[6].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[7].Address        =   232;                                     // DCC Address 232 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[7].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[7].Position1      =     7;                                     // Turntable Track 7
  DCC_Accessory[7].Position2      =     8;                                     // Turntable Track 8
  DCC_Accessory[7].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[7].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[7].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[7].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[7].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[7].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[7].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[8].Address        =   233;                                     // DCC Address 233 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[8].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[8].Position1      =     9;                                     // Turntable Track 9
  DCC_Accessory[8].Position2      =    10;                                     // Turntable Track 10
  DCC_Accessory[8].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[8].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[8].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[8].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[8].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[8].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[8].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[9].Address        =   234;                                     // DCC Address 234 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[9].Button         =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[9].Position1      =    11;                                     // Turntable Track 11
  DCC_Accessory[9].Position2      =    12;                                     // Turntable Track 12
  DCC_Accessory[9].OutputPin1     =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[9].OutputPin2     =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[9].ReverseTrack1  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[9].ReverseTrack2  =     0;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[9].Finished       =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[9].Active         =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[9].durationMilli  =   250;                                     // Pulse Time in ms

  DCC_Accessory[10].Address       =   235;                                     // DCC Address 235 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[10].Button        =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[10].Position1     =    31;                                     // Turntable Track 31
  DCC_Accessory[10].Position2     =    32;                                     // Turntable Track 32
  DCC_Accessory[10].OutputPin1    =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[10].OutputPin2    =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[10].ReverseTrack1 =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[10].ReverseTrack2 =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[10].Finished      =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[10].Active        =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[10].durationMilli =   250;                                     // Pulse Time in ms

  DCC_Accessory[11].Address       =   236;                                     // DCC Address 236 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[11].Button        =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[11].Position1     =    33;                                     // Turntable Track 33
  DCC_Accessory[11].Position2     =    34;                                     // Turntable Track 34
  DCC_Accessory[11].OutputPin1    =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[11].OutputPin2    =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[11].ReverseTrack1 =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[11].ReverseTrack2 =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[11].Finished      =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[11].Active        =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[11].durationMilli =   250;                                     // Pulse Time in ms

  DCC_Accessory[12].Address       =   237;                                     // DCC Address 237 0 = Goto Position1 , 1 = Position2
  DCC_Accessory[12].Button        =     0;                                     // Accessory Button: 0 = OFF (Red), 1 = ON (Green)
  DCC_Accessory[12].Position1     =    35;                                     // Turntable Track 35
  DCC_Accessory[12].Position2     =    36;                                     // Turntable Track 36
  DCC_Accessory[12].OutputPin1    =    14;                                     // Arduino Output Pin 14 = Red LED
  DCC_Accessory[12].OutputPin2    =    15;                                     // Arduino Output Pin 15 = Green LED
  DCC_Accessory[12].ReverseTrack1 =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[12].ReverseTrack2 =     1;                                     // Reverse Track Power: 0 = Normal, 1 = Reversed
  DCC_Accessory[12].Finished      =     1;                                     // Command Busy = 0 or Finished = 1
  DCC_Accessory[12].Active        =     0;                                     // Command Not Active = 0, Active = 1
  DCC_Accessory[12].durationMilli =   250;                                     // Pulse Time in ms

} // END DCC_Accessory_ConfigureDecoderFunctions


void DCC_Accessory_CheckStatus()
{
  for (uint8_t AccDec = 0; AccDec < DCC_Max_Accessories; AccDec++)                 // Begin loop through DCC Accessory Decoders
  {
    DCC.loop();                                                                // Loop DCC Library
    if (DCC_Accessory[AccDec].Finished && DCC_Accessory[AccDec].Active)
    {
      DCC_Accessory[AccDec].Finished = 0;
      DCC_Accessory[AccDec].offMilli = millis() + DCC_Accessory[AccDec].durationMilli;
      if (DCC_Accessory[AccDec].Address >= 228)
      {
//        Serial.println(F("------------------------------------------------------------------------------------------------------------------------------"));
      } // END if
      Serial.print(F("Address: "));
      Serial.print(DCC_Accessory[AccDec].Address);
      Serial.print(F(", "));
      Serial.print(F("Button: "));
      Serial.print(DCC_Accessory[AccDec].Button);
      Serial.print(F(" ("));
      Serial.print((DCC_Accessory[AccDec].Button) ? "Green" : "Red");          // 0 = Red, 1 = Green
      Serial.println(F(")"));
      switch (DCC_Accessory[AccDec].Address)
      {
        case (225):                                                            // DCC Address 225 0 = END, 1 = INPUT
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button    : 0 = END
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin1;                 // Set Arduino Output Pin - case 225 0 = END
            Turntable_NewTrack = Turntable_CurrentTrack;                       // Stop at Current Track
            Turntable_OldAction = STOP;                                        // Action: Stop Motor
            Turntable_NewAction = DCC_END;                                     // Action: Stop Motor
            Turntable_Action = DCC_END;                                        // Requested Action = DCC_END
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button  : 1 = INPUT
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin - case 225 1 = INPUT
            Turntable_OldAction = STOP;                                        // Action: Stop Motor
            Turntable_NewAction = DCC_INPUT;                                   // Action: Stop Motor
            Turntable_Action = DCC_INPUT;                                      // Requested Action = DCC_INPUT
          } // END if
          break; // END case 225
  
        case (226):                                                            // DCC Address 226 0 = CLEAR, 1 = TURN 180
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button    : 0 = CLEAR
          {
            Turntable_CurrentTrack = 1;                                        // Bridge in Home Position
            Turntable_NewTrack = 1;                                            // Bridge in Home Position
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin1;                 // Set Arduino Output Pin - case 226 0 = CLEAR
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_CLEAR;                                   // Action: ???
            Turntable_Action = DCC_CLEAR;                                      // Requested Action = DCC_CLEAR
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button  : 1 = TURN 180
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin - case 226 1 = TURN 180
            if (Turntable_CurrentTrack < 19)
            {
              Turntable_NewTrack = Turntable_CurrentTrack + (maxTrack / 2);
            } // END if
            else
            {
              Turntable_NewTrack = Turntable_CurrentTrack - (maxTrack / 2);
            } // END else
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_T180;                                    // Action: Turn Motor (maxTrack / 2) Steps
            Turntable_Action = DCC_T180;                                       // Requested Action = DCC_T180
          } // END if
          break; // END case 226
  
        case (227):                                                            // DCC Address 227 0 = 1 STEP CW, 1 = 1 STEP CCW
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button   : 0 = Turn 1 Step ClockWise
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin1;                 // Set Arduino Output Pin - case 227 0 = 1 STEP CW
            Turntable_NewTrack = Turntable_CurrentTrack + 1;
            if (Turntable_NewTrack > maxTrack)                                 // From Track 36 to Track 1
            {
              Turntable_NewTrack = 1;                                          // Track (1)
            } // END if
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_T1CW;                                    // Action: Turn Motor 1 Step ClockWise
            Turntable_Action = DCC_T1CW;                                       // Requested Action = DCC_T1CW
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button : 1 = Turn 1 Step Counter ClockWise
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin - case 227 1 = 1 STEP CCW
            Turntable_NewTrack = Turntable_CurrentTrack - 1;
            if (Turntable_NewTrack == 0)                                       // From Track 1 to Track 36
            {
              Turntable_NewTrack = maxTrack;                                   // Track (maxTrack)
            } // END if
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_T1CCW;                                   // Action: Turn Motor 1 Step Counter ClockWise
            Turntable_Action = DCC_T1CCW;                                      // Requested Action = DCC_T1CCW
          } // END if
          break; // END case 227
  
        case (228):                                                            // DCC Address 228 0 = Direction CW, 1 = Direction CCW
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button   : 0 = Direction CW
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin1;                 // Set Arduino Output Pin - case 228 0 = Direction CW
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_DCW;                                     // Action: Motor Direction ClockWise
            Turntable_Action = DCC_DCW;                                        // Requested Action = DCC_DCW
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button : 1 = Direction CCW 0 = Direction CW
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin - case 228 1 = Direction CCW
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            Turntable_NewAction = DCC_DCCW;                                    // Action: Motor Direction Counter ClockWise
            Turntable_Action = DCC_DCCW;                                       // Requested Action = DCC_DCCW
          } // END if
          break; // END case 228
  
        default:                                                               // DCC Address 229 to DCC_Max_Accessories
          if (DCC_Accessory[AccDec].Button == 0)                               // Red Button   : 0 = Goto Track Position1
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin1;                 // Set Arduino Output Pin - case default 0 = Red
            Turntable_NewTrack = DCC_Accessory[AccDec].Position1;              // Set New Turntable Track from DCC Address
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            SetDirection();                                                    // Determine Direction ClockWise or Counter ClockWise
            Turntable_Action = Turntable_NewAction;                            // Requested Action = Depends on SetDirection
          } // END if
  
          if (DCC_Accessory[AccDec].Button == 1)                               // Green Button : 1 = Goto Track Position2
          {
            DCC_Action_LED = DCC_Accessory[AccDec].OutputPin2;                 // Set Arduino Output Pin - case default 1 = Green
            Turntable_NewTrack = DCC_Accessory[AccDec].Position2;              // Set New Turntable Track from DCC Address
            Turntable_OldAction = Turntable_NewAction;                         // Switch Old Action
            SetDirection();                                                    // Determine Direction ClockWise or Counter ClockWise
            Turntable_Action = Turntable_NewAction;                            // Requested Action = Depends on SetDirection
          } // END if
          break; // END default
  
      } // END switch

  //                  0123456789012345678901234                                // Sample text
      Serial.print(F("DCC_ACC_Status       --> "));                            // Serial print Function
      PrintStatus();                                                           // Print Actions and Track Numbers
      Serial.println(F("------------------------------------------------------------------------------------------------------------------------------"));
      LCDPrintTrackStatus();                                                   // LCD print text
    } // END if
    
    if ((!DCC_Accessory[AccDec].Finished) && (millis() > DCC_Accessory[AccDec].offMilli))
    {
      DCC_Accessory[AccDec].Finished = 1;
      DCC_Accessory[AccDec].Active = 0;
    } // END if
  } // END for                                                                 // End loop through DCC Accessory Decoders
} // END DCC_Accessory_CheckStatus


void Button_CheckStatus()
{
  Button_T180.loop();                                                          // Check debounce and update the state of the button
  Button_Right.loop();                                                         // Check debounce and update the state of the button
  Button_Left.loop();                                                          // Check debounce and update the state of the button
  Button_T180_CurrentState = Button_T180.getState();                           // HIGH = Released, LOW = Pressed
  if (Button_T180_LastState == HIGH && Button_T180_CurrentState == LOW)        // LOW = Pressed
  {
    Button_T180_IsPressed = true;
    Button_T180_IsPressing = true;
    Button_T180_ShortPressed = false;
    Button_T180_LongPressed = false;
    Button_T180_PressedTime = millis();
//    Serial.println();
//    Serial.print("Button_T180_PressedTime: ");
//    Serial.println(Button_T180_PressedTime);
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
//      Serial.println("Button Released.");
      Button_T180_LongPressed = false;
    } // END else if
  } // END else if
  if (Button_T180_IsPressing && !Button_T180_LongPressed)
  {
    Button_T180_PressTime = millis() - Button_T180_PressedTime;
    if (Button_T180_PressTime >= Button_T180_LongPressTime)
    {
      Button_T180_LongPressed = true;
    }
  }
  if (Button_T180_ShortPressed)                                                // Button T180 Pressed : TURN 180
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
    Turntable_NewAction = BUT_T180;                                            // Action: Turn Motor (maxTrack / 2) Steps
    Turntable_Action = BUT_T180;                                               // Requested Action = BUT_T180
    Serial.println();                                                          // Serial print Function
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("ShortPressTime: "));                                       // Serial print Function
    Serial.print(Button_T180_PressTime);                                       // Serial print Function
    if (Button_T180_PressTime < 100)
    {
      Serial.print(F("   --> "));                                              // Serial print Function
    } // END if
    else if (Button_T180_PressTime < 1000)
    {
      Serial.print(F("  --> "));                                               // Serial print Function
    } // END if
    else
    {
      Serial.print(F(" --> "));                                                // Serial print Function
    } // END if
    PrintStatus();                                                             // Print Actions and Track Numbers
  }
  if (Button_T180_IsPressed && Button_T180_LongPressed)                        // Button T180 Long Pressed : Store current position as track 1
  {
    Button_T180_IsPressed = false;
    DCC_Action_LED = 16;                                                       // Set Arduino Output Pin 16 = Yellow LED
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_STORE;                                           // Action: Store current position as track 1
    Turntable_Action = BUT_STORE;                                              // Requested Action = BUT_STORE
    Serial.println();                                                          // Serial print Function
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("LongPressTime: "));                                        // Serial print Function
    Serial.print(Button_T180_PressTime);                                       // Serial print Function
    Serial.print(F("  --> "));                                                 // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  }
  Button_T180_LastState = Button_T180_CurrentState;
  
  if (Button_Right.isPressed())                                                // Button Right Pressed : Turn 1 Step ClockWise
  {
    DCC_Action_LED = 14;                                                       // Set Arduino Output Pin 14 = Red LED
    Turntable_NewTrack = Turntable_CurrentTrack + 1;
    if (Turntable_NewTrack > maxTrack)                                         // From Track 36 to Track 1
    {
      Turntable_NewTrack = 1;                                                  // Track (1)
    } // END if
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_T1CW;                                            // Action: Turn Motor 1 Step ClockWise
    Turntable_Action = BUT_T1CW;                                               // Requested Action = BUT_T1CW
  //                0123456789012345678901234                                  // Sample text
    Serial.println();
    Serial.print(F("BUT_T1CW Status      --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if (Button_Left.isPressed())                                                 // Button Left Pressed: Turn 1 Step Counter ClockWise
  {
    DCC_Action_LED = 15;                                                       // Set Arduino Output Pin 15 = Green LED
    Turntable_NewTrack = Turntable_CurrentTrack - 1;
    if (Turntable_NewTrack == 0)                                               // From Track 1 to Track 36
    {
      Turntable_NewTrack = maxTrack;                                           // Track (maxTrack)
    } // END if
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = BUT_T1CCW;                                           // Action: Turn Motor 1 Step Counter ClockWise
    Turntable_Action = BUT_T1CCW;                                              // Requested Action = BUT_T1CCW
  //                0123456789012345678901234                                  // Sample text
    Serial.println();
    Serial.print(F("BUT_T1CCW Status     --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
} // END Button_CheckStatus


void Turntable_CheckSwitch()                                                   // From HIGH to LOW = Bridge in Position
{
  Turntable_Switch.loop();                                                     // Check debounce and update the state of the Turntable Switch
  if (Turntable_Switch.isPressed())                                            // Bridge in Position
  {
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = POS;                                                 // Bridge in Position
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("TT_CheckSwitch       --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
} // END Turntable_CheckSwitch


void Turntable_StoreTrack()                                                    // Store Track Position in EEPROM
{
  digitalWrite(Turntable_StatusLED, HIGH);                                     // Set Arduino Onboard LED 17 = Blue LED = Bridge in Position ON
  digitalWrite(DCC_Action_LED     , LOW);                                      // LED OFF
  EEPROM.update(EE_Address, Turntable_CurrentTrack);                           // Store Turntable bridge position into EEPROM
  Serial.print(F("Track "));                                                   // Serial print Function
  Serial.print(Turntable_CurrentTrack);                                        // Serial print Function
  Serial.print(F(" stored at EEPROM address "));                               // Serial print Function
  Serial.println(EE_Address);                                                  // Serial print Function
} // END Turntable_StoreTrack


void Turntable_CheckPos()                                                      // Check if Bridge on wanted track position
{
  if (Turntable_CurrentTrack == Turntable_NewTrack)                            // Bridge in Position = Stop Motor
  {
    switch (Turntable_OldAction)                                               // Check OldAction
    {
      case MCW:                                                                // Turntable turning ClockWise
        speedValue = 0;                                                        // Zero = Direction Stop - MCW
        Turntable_SetMotorSpeed(speedValue);                                   // Motor Stop - MCW
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = STOP;                                            // Action: STOP
  //                    0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos MCW      --> "));                          // Serial print Function
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCW
        
      case MCCW:                                                               // Turntable turning Counter ClockWise
        speedValue = 0;                                                        // Zero = Direction Stop - MCCW
        Turntable_SetMotorSpeed(speedValue);                                   // Motor Stop - MCCW
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = STOP;                                            // Set New Action: STOP
  //                    0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos MCCW     --> "));                          // Serial print Function
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCCW
        
      default:                                                                 // None of the above
  //                    0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos "));                                       // Serial print Function
        Serial.print(Turntable_OldAction);                                     // Print OldAction
        Serial.print(F(" --> "));                                              // Serial print Function
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
        Turntable_NewAction = MCW;                                             // Action: MCW
  //                    0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos MCW      --> "));                          // Serial print Function
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCW
        
      case MCCW:                                                               // Turntable turning Counter ClockWise
        digitalWrite(Turntable_StatusLED, LOW);                                // Set Arduino Onboard LED 17 = Blue LED = Bridge in Position OFF
        digitalWrite(DCC_Action_LED  , HIGH);                                  // Set DCC Action LED ON
        Turntable_OldAction = Turntable_NewAction;                             // Switch Old Action
        Turntable_NewAction = MCCW;                                            // Set New Action: MCCW
  //                    0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos MCCW     --> "));                          // Serial print Function
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END case MCCW
        
      default:                                                                 // None of the above
  //                    0123456789012345678901234                              // Sample text
        Serial.print(F("TT_CheckPos "));                                       // Serial print Function
        Serial.print(Turntable_OldAction);                                     // Print OldAction
        Serial.print(F(" --> "));                                              // Serial print Function
        PrintStatus();                                                         // Print Actions and Track Numbers
        break; // END default
        
    } // END switch
  } // END else
} // END Turntable_CheckPos


void DCC_Accessory_LED_OFF()                                                   // All LEDs OFF
{
  for (uint8_t AccDec = 0; AccDec < DCC_Max_Accessories; AccDec++)
  {
    digitalWrite(DCC_Accessory[AccDec].OutputPin1, LOW);                       // LED OFF
    digitalWrite(DCC_Accessory[AccDec].OutputPin2, LOW);                       // LED OFF
  } // END for
} // END DCC_Accessory_LED_OFF


void DCC_Action_LED_Startup()                                                  // All DCC Action LEDs ON and OFF
{
  for (int DCC_Action_LED = 14; DCC_Action_LED <= 17; DCC_Action_LED++)        // Loop DCC Action LED
  {                                                                            // Short LED test at startup
    digitalWrite(DCC_Action_LED, HIGH);                                        // Switch LED ON
    delay(200);                                                                // LED ON for 200 msec
    digitalWrite(DCC_Action_LED, LOW);                                         // Switch LED OFF
  } // END for
} // END DCC_Action_LED_Startup


void Turntable_Init()                                                          // Start Initialize Turntable Procedure
{
  digitalWrite(Turntable_StatusLED, LOW);                                      // Set Arduino Onboard LED 17 = Blue LED = Bridge in Position OFF
  digitalWrite(DCC_Action_LED, HIGH);                                          // Set DCC Action LED ON
} // END Turntable_Init


void Turntable_Clear()                                                         // Simple Blink Action to comfirm DCC_CLEAR
{
  digitalWrite(DCC_Action_LED, LOW);                                           // Set DCC Action LED OFF
  delay(200);
  digitalWrite(DCC_Action_LED, HIGH);                                          // Set DCC Action LED ON
  delay(200);
  digitalWrite(DCC_Action_LED, LOW);                                           // Set DCC Action LED OFF
  delay(200);
  digitalWrite(DCC_Action_LED, HIGH);                                          // Set DCC Action LED ON
  delay(200);
  digitalWrite(DCC_Action_LED, LOW);                                           // Set DCC Action LED OFF
  delay(1000);
  digitalWrite(DCC_Action_LED, HIGH);                                          // Set DCC Action LED ON
} // END Turntable_Clear


void Turntable_End()                                                           // End Initialize Turntable Procedure
{
  digitalWrite(Turntable_StatusLED, LOW);                                      // Set Arduino Output Pin 17 = Blue LED = Bridge in Position OFF
  digitalWrite(DCC_Action_LED, LOW);                                           // Set DCC Action LED OFF
} // END Turntable_End

void BridgeRelayInit()                                                         //  Initialize Reverse Tracks
{
  Serial.println(F("Initialize Reverse Tracks (0 = Normal, 1 = Reversed)"));
  for (uint8_t AccDec = 4; AccDec < DCC_Max_Accessories; AccDec++)             // DCC Accessory Decoders 0..3 are special functions. Start at #4
  {                                                                            // Configure Reverse Track (false = Normal, true = Reversed)
    Track1 = DCC_Accessory[AccDec].Position1;
    DCC_ReverseTrack[Track1] = DCC_Accessory[AccDec].ReverseTrack1;            // Set Reverse Track Status (false = Normal, true = Reversed)
    Serial.print(F("Track "));
    if (Track1 < 10)
    {
    Serial.print(F(" "));
    }
    Serial.print(Track1);
    Serial.print(F(": "));
    Serial.print(DCC_ReverseTrack[Track1]);
    Serial.print(F(",  Track "));
    if (Track1 <= 18)
    {
      DCC_ReverseTrack[Track1 + 18] = !DCC_Accessory[AccDec].ReverseTrack1;    // Set Reverse Track Status (false = Normal, true = Reversed)
      Serial.print(Track1 + 18);
      Serial.print(F(": "));
      Serial.print(DCC_ReverseTrack[Track1 + 18]);
    } // END if
    else
    {
      DCC_ReverseTrack[Track1 - 18] = !DCC_Accessory[AccDec].ReverseTrack1;    // Set Reverse Track Status (false = Normal, true = Reversed)
      Serial.print(Track1 - 18);
      Serial.print(F(": "));
      Serial.print(DCC_ReverseTrack[Track1 - 18]);
    } // END else
    Serial.println();

    Track2 = DCC_Accessory[AccDec].Position2;
    DCC_ReverseTrack[Track2] = DCC_Accessory[AccDec].ReverseTrack2;            // Set Reverse Track Status (false = Normal, true = Reversed)
    Serial.print(F("Track "));
    if (Track2 < 10)
    {
    Serial.print(F(" "));
    }
    Serial.print(Track2);
    Serial.print(F(": "));
    Serial.print(DCC_ReverseTrack[Track2]);
    Serial.print(F(",  Track "));
    if (Track2 <= 18)
    {
      DCC_ReverseTrack[Track2 + 18] = !DCC_Accessory[AccDec].ReverseTrack2;    // Set Reverse Track Status (false = Normal, true = Reversed)
      Serial.print(Track2 + 18);
      Serial.print(F(": "));
      Serial.print(DCC_ReverseTrack[Track2 + 18]);
    } // END if
    else
    {
      DCC_ReverseTrack[Track2 - 18] = !DCC_Accessory[AccDec].ReverseTrack2;    // Set Reverse Track Status (false = Normal, true = Reversed)
      Serial.print(Track2 - 18);
      Serial.print(F(": "));
      Serial.print(DCC_ReverseTrack[Track2 - 18]);
    } // END else
    Serial.println();
  } // END for
} // END BridgeRelayInit()


void BridgeRelayCheck()                                                        // Check Bride Relay Status = ULN2803A
{
  if ((BridgeRelayActive) && (millis() > BridgeRelayMaxMillis))                // Check Bridge Relay Active Timer
  {
    BridgeRelayActive = false;                                                 // Deactivate Bridge Relay Set
    BridgeRelayOff();                                                          // Disable Outputs to ULN2803A
  } // END if
  
} // END BridgeRelayCheck


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

void BridgeRelayOff()                                                          // Disable Outputs to ULN2803A
{
  digitalWrite(BridgeRelayL1, LOW);                                            // Arduino Output Pin 7 OFF = ULN2803A Pin 1+2       = Bridge Relay L1-
  digitalWrite(BridgeRelayL2, LOW);                                            // Arduino Output Pin 8 OFF = ULN2803A Pin 3+4       = Bridge Relay L2-
  Serial.println(F("Bridge Relay: Off     "));                                 // Serial print Function
} // END BridgeRelayOff


void BridgeRelayOnNormal()                                                     // Enable Outputs to ULN2803A - Normal
{
  digitalWrite(BridgeRelayL1, HIGH);                                           // Arduino Output Pin 7 ON  = ULN2803A Pin 1+2       = Bridge Relay L1-
  digitalWrite(BridgeRelayL2, LOW);                                            // Arduino Output Pin 8 OFF = ULN2803A Pin 3+4       = Bridge Relay L2-
  //              0123456789012345678901234                                    // Sample text
  Serial.println(F("Bridge Relay: Normal     "));                              // Serial print Function
} // END BridgeRelayOnNormal


void BridgeRelayOnReversed()                                                   // Enable Outputs to ULN2803A - Reversed
{
  digitalWrite(BridgeRelayL1, LOW);                                            // Arduino Output Pin 7 OFF = ULN2803A Pin 1+2       = Bridge Relay L1-
  digitalWrite(BridgeRelayL2, HIGH);                                           // Arduino Output Pin 8 ON  = ULN2803A Pin 3+4       = Bridge Relay L2-
  //              0123456789012345678901234                                    // Sample text
  Serial.println(F("Bridge Relay: Reversed   "));                              // Serial print Function
} // END BridgeRelayOnReversed


void Turntable_SetMotorSpeed(int speed)                                        // Set Motor Speed
{
  DCC_Action_LED_Reset();                                                      // DCC_Action_LED OFF
  digitalWrite(DCC_Action_LED  , HIGH);                                        // LED ON
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
      TurntableLockResetLock();                                                // Enable Outputs to L293D - Lock Free
      digitalWrite(Turntable_MotorM2, LOW);                                    // Output T.M2 L293D OFF
      analogWrite(Turntable_MotorM1, speed);                                   // PWM on T.M1 L293D
      BridgeRelayMaxMillis = millis() + BridgeRelayPulsTime;                   // Set Bridge Relay Active Timer
      BridgeRelayActive = true;                                                // Activate Bridge Relay Set
      break; // END case maxSpeed
    case (-maxSpeed):                                                          // Motor Reverse
      TurntableLockResetLock();                                                // Enable Outputs to L293D - Lock Free
      digitalWrite(Turntable_MotorM1, LOW);                                    // Output T.M1 L293D OFF
      analogWrite(Turntable_MotorM2, -speed);                                  // PWM on T.M2 L293D
      BridgeRelayMaxMillis = millis() + BridgeRelayPulsTime;                   // Set Bridge Relay Active Timer
      BridgeRelayActive = true;                                                // Activate Bridge Relay Set
      break; // END case -maxSpeed
  } // END switch
//LCDPrintTrackText();                                                         // LCD print text
  LCDPrintTrackStatus();                                                       // LCD print text
} // END Turntable_SetMotorSpeed


void GetDirection()                                                            // Determine Direction based on Zones
{
  uint8_t FromZone, ToZone;
  FromZone = GetZone( Turntable_CurrentTrack - 1 );
  ToZone = GetZone( Turntable_NewTrack - 1);
  Serial.print(F("From Zone: "));                                              // Serial print Function
  Serial.println(FromZone);
  Serial.print(F("To Zone: "));                                                // Serial print Function
  Serial.println(ToZone);

  //     FromZone
  //    0  1  2  3
// T    |  |  |  |
// o 0-{0, 1, 1, 0}
// Z 1-{0, 0, 1, 0}
// o 2-{0, 0, 0, 1}
// n 3-{1, 0, 0, 0}
// e

  if ( ToZone != FromZone )                                                    // Only when FromZone not the same as ToZone
  {
    if ( Directions[ToZone][FromZone] )                                        // Directions matrix contains 1 = CCW
    {
      Serial.println(F("Directions CCW"));                                     // Serial print Function
      Turntable_NewAction = TCCW;                                              // Action: Turn Motor M1 Counter ClockWise
      speedValue = -maxSpeed;                                                  // Negative = Direction Counter ClockWise
    } // END if
    else
    {
      Serial.println(F("Directions CW"));                                      // Serial print Function
      Turntable_NewAction = TCW;                                               // Action: Turn Motor M1 ClockWise
      speedValue = maxSpeed;                                                   // Positive = Direction ClockWise
    } // END else
  } // END if
  else                                                                         // FromZone same as ToZone
  {
    if ( Turntable_NewTrack < Turntable_CurrentTrack )
    {
      Serial.println(F("In Zone CCW"));                                        // Serial print Function
      Turntable_NewAction = TCCW;                                              // Action: Turn Motor M1 Counter ClockWise
      speedValue = -maxSpeed;                                                  // Negative = Direction Counter ClockWise
    } // END if
    else
    {
      Serial.println(F("In Zone CW"));                                         // Serial print Function
      Turntable_NewAction = TCW;                                               // Action: Turn Motor M1 ClockWise
      speedValue = maxSpeed;                                                   // Positive = Direction ClockWise
    } // END else
  } // END else
} // END GetDirection


void SetDirection()                                                            // Set Direction based Action or Zones
{
  if (Turntable_NewTrack == Turntable_CurrentTrack)                            // Check Current Track
  {
    Turntable_NewAction = STOP;                                                // Action: STOP
    speedValue = 0;                                                            // Zero = Direction Stop - Stop
  } // END if
  else if (Turntable_OldAction == DCC_DCW)                                     // Check Old Action
  {
    Turntable_NewAction = TCW;                                                 // Action: Turn Motor ClockWise
    speedValue = maxSpeed;                                                     // Positive = Direction ClockWise - TCW
  } // END else if
  else if (Turntable_OldAction == DCC_DCCW)                                    // Check Old Action
  {
    Turntable_NewAction = TCCW;                                                // Action: Turn Motor Counter ClockWise
    speedValue = -maxSpeed;                                                    // Negative = Direction Counter ClockWise - TCCW
  } // END else if
  else                                                                         // Old Action unknown (probably STOP)
  {
    GetDirection();                                                            // Determine Direction based on Zones
  } // END else
  Serial.print(F("SetDirection         --> "));                                // Serial print Function
  PrintStatus();                                                               // Print Actions and Track Numbers
} // END SetDirection


uint8_t GetZone(uint8_t Track )                                                // Detemine Zone from Track
{
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


void DCC_Action_LED_Reset()                                                    // DCC Action LEDs OFF
{
  for (int ledpin = 14; ledpin <= 17; ledpin++)                                // Loop ledpin
  {
    digitalWrite(ledpin, LOW);                                                 // Switch LED OFF
  } // END for
}

void PrintStatus()                                                             // Serial print status
{
  Serial.print(Turntable_States[Turntable_Action]);                            // Serial print action
  Serial.print(F(": Old: "));                                                  // Serial print text
  Serial.print(Turntable_States[Turntable_OldAction]);                         // Serial print action
  Serial.print(F(", New: "));                                                  // Serial print text
  Serial.print(Turntable_States[Turntable_NewAction]);                         // Serial print action
  Serial.print(F(", Current: "));                                              // Serial print text
  if (Turntable_CurrentTrack < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  }
  Serial.print(Turntable_CurrentTrack);                                        // Serial print value
  Serial.print(F(", NewTrack: "));                                             // Serial print text
  if (Turntable_NewTrack < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  }
  Serial.print(Turntable_NewTrack);                                            // Serial print value
  Serial.print(F(", DCC_Action_LED: "));                                       // Serial print text
  if (DCC_Action_LED < 10)
  {
    Serial.print(F(" "));                                                      // Serial print text
  }
  Serial.print(DCC_Action_LED);                                                // Serial print value
  Serial.print(F(", Speed: "));                                                // Serial print text
  if (speedValue == 0)
  {
    Serial.print(F("   "));                                                    // Serial print text
  }
  if (speedValue > 0)
  {
    Serial.print(F(" "));                                                      // Serial print text
  }
  Serial.print(speedValue);                                                    // Serial print value
  Serial.println();                                                            // Serial print new line
} // END PrintStatus


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


void LCDPrintTrackText()                                                       // LCD print text
{
  lcd.setCursor(0, 3);                                                         // Set cursor to fourth line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("Track:    to        "));                                        // LCD print text
} // END LCDPrintTrackText


void LCDPrintAction()
{
  lcd.setCursor(10, 1);                                                        // Set cursor to second line and 10th character
  lcd.print(Turntable_States[Turntable_Action]);                               // LCD print action
  lcd.setCursor(10, 3);                                                        // Set cursor to fourth line and 10th character
  lcd.print(Turntable_States[Turntable_NewAction]);                            // LCD print text

  lcd.setCursor(0, 3);                                                         // Set cursor to fourth line and left corner
  lcd.print(Turntable_States[Turntable_OldAction]);                            // LCD print text
  lcd.setCursor(10, 3);                                                        // Set cursor to fourth line and 10th character
  lcd.print(Turntable_States[Turntable_NewAction]);                            // LCD print text
} // END LCDPrintAction


void LCDPrintClear()
{
  lcd.setCursor(0, 0);                                                         // Set cursor to first line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("                    "));                                        // Clear text
  lcd.setCursor(0, 1);                                                         // Set cursor to second line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("                    "));                                        // Clear text
  lcd.setCursor(0, 2);                                                         // Set cursor to third line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("                    "));                                        // Clear text
  lcd.setCursor(0, 3);                                                         // Set cursor to fourth line and left corner
  //           01234567890123456789                                            // Sample text
  lcd.print(F("                    "));                                        // Clear text
}


void TurntableLockCheck()                                                      // Check Turntable Lock Status
{
  if ((TurntableLockActive) && (millis() > TurntableLockMillis))               // Check  Turntable Lock Active Timer
  {
    TurntableLockActive = false;                                               // Deactivate Turntable Lock
    TurntableLockOff();                                                        // Disable Outputs to L293D
  } // END if
  
} // END TurntableLockCheck


void TurntableLockOff()                                                        // Disable Outputs to L293D
{
  TurntableLockActive = false;
  digitalWrite(Turntable_LockL1, LOW);                                         // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  digitalWrite(Turntable_LockL2, LOW);                                         // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  Serial.println(F("Turntable Lock Off"));                                     // Serial print Function
  Serial.println(F("------------------------------------------------------------------------------------------------------------------------------"));
} // END TurntableLockOff


void TurntableLockResetLock()                                                  // Enable Outputs to L293D - Lock Free
{
  digitalWrite(Turntable_LockL1, HIGH);                                        // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  digitalWrite(Turntable_LockL2, LOW);                                         // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  TurntableLockMillis = millis() + TurntableLockInterval;                      // Set Turntable Lock Active Timer
  TurntableLockActive = true;
  Serial.println(F("Turntable Lock Free"));                                    // Serial print Function
} // END TurntableLockFree


void TurntableLockSetLock()                                                    // Enable Outputs to L293D - Lock Active
{
  digitalWrite(Turntable_LockL1, LOW);                                         // Arduino Output Pin 10 = Turntable Lock L1   = Turntable Cable Pin 5
  digitalWrite(Turntable_LockL2, HIGH);                                        // Arduino Output Pin  9 = Turntable Lock L2   = Turntable Cable Pin 6
  Serial.println(F("Turntable Lock Active"));                                  // Serial print Function
  TurntableLockMillis = millis() + TurntableLockInterval;                      // Set Turntable Lock Active Timer
  TurntableLockActive = true;
} // END TurntableLockLocked


//-----------------------------------------------------------------------------//
void loop()                                                                    // Arduino Main Program
{
  unsigned long currentMillis = millis();
  if ( currentMillis - WatchdogMillis >= WatchdogInterval )
  {
    WatchdogMillis = currentMillis;
    WatchdogState = !WatchdogState;
    digitalWrite(WatchdogLED, WatchdogState);
  }
  DCC_Accessory_CheckStatus();                                                 // Check DCC Accessory Status
  Button_CheckStatus();                                                        // Check Button Status
  Turntable_CheckSwitch();                                                     // Check Kato Turntable Pin 1
  BridgeRelayCheck();                                                          // Check Bride Relay Status = ULN2803A
  TurntableLockCheck();                                                        // Check Turntable Lock Status
  if ((Turntable_OldAction == STOP) && (Turntable_NewAction == DCC_INPUT))
  {
    Turntable_Init();                                                          // Action: Initialize Turntable
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = DCC_INPUT;                                           // Action: Initialize Turntable
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)DCC_INPUT         -> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == STOP) && (Turntable_NewAction == DCC_CLEAR))
  {
    Turntable_Clear();                                                         // Action: Set Turntable Track to 1
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = DCC_CLEAR;                                           // Action: Set Turntable Track to 1
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)DCC_CLEAR        --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == STOP) && (Turntable_NewAction == DCC_END))
  {
    Turntable_End();                                                           // End all actions
    Turntable_OldAction = STOP;                                                // 
    Turntable_NewAction = STOP;                                                // 
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)DCC_END          --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != DCC_T180) && (Turntable_NewAction == DCC_T180))
  {
    if ((Turntable_CurrentTrack >= 1) && (Turntable_CurrentTrack <= 18))
    {
      speedValue = maxSpeed;                                                   // Positive = Direction ClockWise - DCC_T180
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Forward - DCC_T180
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCW;                                               // Action: Move Motor ClockWise
    } // END if
    else
    {
      speedValue = -maxSpeed;                                                  // Negative = Direction Counter ClockWise - DCC_T180
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Backward - DCC_T180
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCCW;                                              // Action: Move Motor Counter ClockWise
    } // END else
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)DCC_T180         --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != DCC_T1CW) && (Turntable_NewAction == DCC_T1CW))
  {
    speedValue = maxSpeed;                                                     // Positive = Direction ClockWise - DCC_T1CW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Forward - DCC_T1CW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCW;                                                 // Action: Move Motor ClockWise
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)Check T1CW       --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != DCC_T1CCW) && (Turntable_NewAction == DCC_T1CCW))
  {
    speedValue = -maxSpeed;                                                    // Negative = Direction Counter ClockWise - DCC_T1CCW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Reverse - DCC_T1CCW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCCW;                                                // Action: Move Motor Counter ClockWise
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)Check T1CCW      --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != BUT_STORE) && (Turntable_NewAction == BUT_STORE))
  {
    Turntable_CurrentTrack = 1;                                                // Bridge in Home Position
    Turntable_NewTrack = 1;                                                    // Bridge in Home Position
    DCC_Action_LED_Startup();                                                  // All DCC Action LEDs ON and OFF
    Turntable_StoreTrack();                                                    // Store Track Position in EEPROM
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)BUT_STORE        --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if

  if ((Turntable_OldAction != BUT_T180) && (Turntable_NewAction == BUT_T180))
  {
    if ((Turntable_CurrentTrack >= 1) && (Turntable_CurrentTrack <= 18))
    {
      speedValue = maxSpeed;                                                   // Positive = Direction ClockWise - BUT_T180
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Forward - BUT_T180
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCW;                                               // Action: Move Motor ClockWise
    } // END if
    else
    {
      speedValue = -maxSpeed;                                                  // Negative = Direction Counter ClockWise - BUT_T180
      Turntable_SetMotorSpeed(speedValue);                                     // Motor Backward - BUT_T180
      Turntable_OldAction = Turntable_NewAction;                               // Switch Old Action
      Turntable_NewAction = MCCW;                                              // Action: Move Motor Counter ClockWise
    } // END else
    //              0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)BUT_T180         --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != BUT_T1CW) && (Turntable_NewAction == BUT_T1CW))
  {
    speedValue = maxSpeed;                                                     // Positive = Direction ClockWise - BUT_T1CW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Forward - BUT_T1CW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCW;                                                 // Action: Move Motor ClockWise
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)BUT_T1CW         --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != BUT_T1CCW) && (Turntable_NewAction == BUT_T1CCW))
  {
    speedValue = -maxSpeed;                                                    // Negative = Direction Counter ClockWise - BUT_T1CCW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Reverse - BUT_T1CCW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCCW;                                                // Action: Move Motor Counter ClockWise
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)BUT_T1CCW        --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != TCW) && (Turntable_NewAction == TCW))
  {
    speedValue = maxSpeed;                                                     // Positive = Direction ClockWise - TCW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Forward - TCW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCW;                                                 // Action: Move Motor ClockWise
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)Check TCW        --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction != TCCW) && (Turntable_NewAction == TCCW))
  {
    speedValue = -maxSpeed;                                                    // Negative = Direction Counter ClockWise - TCCW
    Turntable_SetMotorSpeed(speedValue);                                       // Motor Reverse - TCCW
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = MCCW;                                                // Action: Move Motor Counter ClockWise
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)Check TCCW       --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if

  if ((Turntable_OldAction == MCW) && (Turntable_NewAction == POS))            // Move ClockWise and Turntable in Position
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack + 1;
    if (Turntable_CurrentTrack > maxTrack)                                     // From Track 36 to Track 1
    {
      Turntable_CurrentTrack = 1;                                              // Track (1)
    } // END if
    Turntable_CheckPos();                                                      // Check if Bridge on wanted track position
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)T+1 - Check MCW  --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if

  if ((Turntable_OldAction == MCCW) && (Turntable_NewAction == POS))           // Move Counter ClockWise and Turntable in Position
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack - 1;
    if (Turntable_CurrentTrack == 0)                                           // From Track 1 to Track 36
    {
      Turntable_CurrentTrack = maxTrack;                                       // Track (maxTrack)
    } // END if
    Turntable_CheckPos();                                                      // Check if Bridge in Position
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)T-1 - Check MCCW --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if

  if ((Turntable_OldAction == DCC_DCW) && (Turntable_NewAction == POS))        // Move ClockWise and Turntable in Position
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack + 1;
    if (Turntable_CurrentTrack > maxTrack)                                     // From Track 36 to Track 1
    {
      Turntable_CurrentTrack = 1;                                              // Track (1)
    } // END if
    Turntable_CheckPos();                                                      // Check if Bridge in Position
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)T+1 - Check DCW  --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if
  
  if ((Turntable_OldAction == DCC_DCCW) && (Turntable_NewAction == POS))       // Move Counter ClockWise and Turntable in Position
  {
    Turntable_CurrentTrack = Turntable_CurrentTrack - 1;
    if (Turntable_CurrentTrack == 0)                                           // From Track 1 to Track 36
    {
      Turntable_CurrentTrack = maxTrack;                                       // Track (maxTrack)
    } // END if
    Turntable_CheckPos();                                                      // Check if Bridge in Position
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)T-1 - Check DCCW --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
    LCDPrintTrackStatus();                                                     // LCD print text
  } // END if
  
  if ((Turntable_OldAction == POS) && (Turntable_NewAction == STOP))           // STOP
  {
    Turntable_StoreTrack();                                                    // Store Track Position in EEPROM
    BridgeRelayActive = true;                                                  // Activate Bridge Relay Set
    BridgeRelaySet();                                                          // Set Bride Relay Status = ULN2803A
    Turntable_OldAction = Turntable_NewAction;                                 // Switch Old Action
    Turntable_NewAction = STOP;                                                // Action: STOP
  //                0123456789012345678901234                                  // Sample text
    Serial.print(F("(..)STOP             --> "));                              // Serial print Function
    PrintStatus();                                                             // Print Actions and Track Numbers
  } // END if
} // END loop
//-----------------------------------------------------------------------------//
