#include <Arduino.h>
#include <NmraDcc.h>
#include <EEPROM.h>
#include <ezButton.h>

namespace
{
constexpr uint8_t PIN_BUTTON_LEFT = 9;
constexpr uint8_t PIN_BUTTON_RIGHT = 10;
constexpr uint8_t PIN_LOCK_L1 = 16;
constexpr uint8_t PIN_LOCK_L2 = 2;
constexpr uint8_t PIN_TURNTABLE_SWITCH = 4;
constexpr uint8_t PIN_DCC_INPUT = 5;
constexpr uint8_t PIN_MOTOR_M1 = 12;
constexpr uint8_t PIN_MOTOR_M2 = 14;

constexpr uint8_t TRACK_COUNT = 36;
constexpr uint16_t BASE_ACCESSORY_ADDRESS = 500;
constexpr uint8_t EEPROM_ADDRESS_CURRENT_TRACK = 0;
constexpr uint32_t LOCK_PULSE_DURATION_MS = 500;
constexpr uint32_t LOCK_SETTLE_DURATION_MS = 600;
constexpr uint32_t UNLOCK_SETTLE_DURATION_MS = 600;
constexpr uint32_t STEP_DEBOUNCE_MS = 60;
constexpr uint32_t BUTTON_MEDIUM_PRESS_MS = 700;
constexpr uint32_t BUTTON_LONG_PRESS_MS = 2000;
constexpr uint32_t BUTTON_COMBO_HOLD_MS = 2000;
constexpr uint8_t MOTOR_PWM_CHANNEL_CW = 0;
constexpr uint8_t MOTOR_PWM_CHANNEL_CCW = 1;
constexpr uint16_t MOTOR_PWM_FREQUENCY = 8000;
constexpr uint8_t MOTOR_PWM_RESOLUTION = 8;
constexpr uint8_t MOTOR_PWM_VALUE = 220;

void writeMotorPwmCw(uint32_t value)
{
#if defined(ESP8266)
  analogWrite(PIN_MOTOR_M1, value);
#else
  ledcWrite(MOTOR_PWM_CHANNEL_CW, value);
#endif
}

void writeMotorPwmCcw(uint32_t value)
{
#if defined(ESP8266)
  analogWrite(PIN_MOTOR_M2, value);
#else
  ledcWrite(MOTOR_PWM_CHANNEL_CCW, value);
#endif
}

struct AccessoryEntry
{
  uint16_t address;
  uint8_t redTrack;
  uint8_t greenTrack;
};

enum class MotorDirection : uint8_t
{
  Idle,
  Clockwise,
  CounterClockwise
};

enum class TurntableState : uint8_t
{
  Idle,
  Unlocking,
  WaitingForUnlock,
  Turning,
  Locking,
  LockSettling
};

enum class ManualLockMode : uint8_t
{
  None,
  Release,
  Engage
};

struct MoveCommand
{
  bool active = false;
  bool isRelative = false;
  uint8_t steps = 0;
  uint8_t targetTrack = 0;
  MotorDirection direction = MotorDirection::Idle;
};

constexpr uint8_t ACCESSORY_COUNT = (TRACK_COUNT / 2) + 1;
AccessoryEntry accessoryTable[ACCESSORY_COUNT];

NmraDcc Dcc;
ezButton buttonLeft(PIN_BUTTON_LEFT);
ezButton buttonRight(PIN_BUTTON_RIGHT);

TurntableState state = TurntableState::Idle;
MotorDirection currentDirection = MotorDirection::Idle;
MoveCommand pendingCommand;
bool currentMoveIsRelative = false;
uint8_t requestedTrack = 0;
uint8_t currentTrack = 0;
uint8_t stepsRemaining = 0;

uint32_t lockDeadline = 0;
uint32_t settleDeadline = 0;
uint32_t lastStepMillis = 0;

bool lastIndexActive = false;

ManualLockMode manualLockMode = ManualLockMode::None;
uint32_t manualLockDeadline = 0;
uint32_t manualLockSettleDeadline = 0;

bool leftButtonHeld = false;
bool rightButtonHeld = false;
bool leftSuppressed = false;
bool rightSuppressed = false;
bool comboActive = false;
bool comboTriggered = false;
uint32_t leftPressedAt = 0;
uint32_t rightPressedAt = 0;
uint32_t comboStartedAt = 0;

void initialiseAccessoryTable()
{
  accessoryTable[0] = {BASE_ACCESSORY_ADDRESS, 0, 0};
  for (uint8_t i = 1; i < ACCESSORY_COUNT; ++i)
  {
    const uint16_t address = BASE_ACCESSORY_ADDRESS + i;
    const uint8_t redTrack = i;
    uint8_t greenTrack = redTrack + (TRACK_COUNT / 2);
    if (greenTrack > TRACK_COUNT)
    {
      greenTrack -= TRACK_COUNT;
    }
    accessoryTable[i] = {address, redTrack, greenTrack};
  }
}

uint8_t advanceTrack(uint8_t track, MotorDirection direction)
{
  if (track < 1 || track > TRACK_COUNT)
  {
    return track;
  }
  switch (direction)
  {
    case MotorDirection::Clockwise:
      return (track == TRACK_COUNT) ? 1 : static_cast<uint8_t>(track + 1);
    case MotorDirection::CounterClockwise:
      return (track == 1) ? TRACK_COUNT : static_cast<uint8_t>(track - 1);
    default:
      return track;
  }
}

uint8_t computeSteps(uint8_t from, uint8_t to, MotorDirection direction)
{
  if (from < 1 || from > TRACK_COUNT || to < 1 || to > TRACK_COUNT || direction == MotorDirection::Idle)
  {
    return 0;
  }
  if (from == to)
  {
    return 0;
  }
  if (direction == MotorDirection::Clockwise)
  {
    return (to > from) ? static_cast<uint8_t>(to - from) : static_cast<uint8_t>((TRACK_COUNT - from) + to);
  }
  return (from > to) ? static_cast<uint8_t>(from - to) : static_cast<uint8_t>(from + (TRACK_COUNT - to));
}

void stopMotor()
{
  writeMotorPwmCw(0);
  writeMotorPwmCcw(0);
  currentDirection = MotorDirection::Idle;
}

void driveMotor(MotorDirection direction)
{
  if (direction == MotorDirection::Clockwise)
  {
    writeMotorPwmCcw(0);
    writeMotorPwmCw(MOTOR_PWM_VALUE);
  }
  else if (direction == MotorDirection::CounterClockwise)
  {
    writeMotorPwmCw(0);
    writeMotorPwmCcw(MOTOR_PWM_VALUE);
  }
  else
  {
    stopMotor();
    return;
  }
  currentDirection = direction;
}

void activateLockReleaseOutputs()
{
  digitalWrite(PIN_LOCK_L1, HIGH);
  digitalWrite(PIN_LOCK_L2, LOW);
}

void activateLockEngageOutputs()
{
  digitalWrite(PIN_LOCK_L1, LOW);
  digitalWrite(PIN_LOCK_L2, HIGH);
}

void setLockRelease()
{
  activateLockReleaseOutputs();
  lockDeadline = millis() + LOCK_PULSE_DURATION_MS;
}

void setLockEngage()
{
  activateLockEngageOutputs();
  lockDeadline = millis() + LOCK_PULSE_DURATION_MS;
}

void lockOutputsOff()
{
  digitalWrite(PIN_LOCK_L1, LOW);
  digitalWrite(PIN_LOCK_L2, LOW);
}

void saveCurrentTrack()
{
  if (currentTrack < 1 || currentTrack > TRACK_COUNT)
  {
    return;
  }
  if (EEPROM.read(EEPROM_ADDRESS_CURRENT_TRACK) != currentTrack)
  {
    EEPROM.write(EEPROM_ADDRESS_CURRENT_TRACK, currentTrack);
    EEPROM.commit();
  }
}

void storeCurrentAsTrackOne()
{
  stopMotor();
  pendingCommand.active = false;
  pendingCommand.isRelative = false;
  pendingCommand.steps = 0;
  pendingCommand.targetTrack = 0;
  pendingCommand.direction = MotorDirection::Idle;
  requestedTrack = 0;
  stepsRemaining = 0;
  currentMoveIsRelative = false;
  currentDirection = MotorDirection::Idle;
  currentTrack = 1;
  saveCurrentTrack();
}

void requestManualLockRelease()
{
  if (state != TurntableState::Idle || manualLockMode != ManualLockMode::None)
  {
    return;
  }
  stopMotor();
  const uint32_t now = millis();
  activateLockReleaseOutputs();
  manualLockMode = ManualLockMode::Release;
  manualLockDeadline = now + LOCK_PULSE_DURATION_MS;
  manualLockSettleDeadline = manualLockDeadline + UNLOCK_SETTLE_DURATION_MS;
}

void requestManualLockEngage()
{
  if (state != TurntableState::Idle || manualLockMode != ManualLockMode::None)
  {
    return;
  }
  stopMotor();
  const uint32_t now = millis();
  activateLockEngageOutputs();
  manualLockMode = ManualLockMode::Engage;
  manualLockDeadline = now + LOCK_PULSE_DURATION_MS;
  manualLockSettleDeadline = 0;
}

void startLockingSequence()
{
  stopMotor();
  setLockEngage();
  state = TurntableState::Locking;
}

void finalizeMovement()
{
  if (!currentMoveIsRelative && requestedTrack >= 1 && requestedTrack <= TRACK_COUNT)
  {
    currentTrack = requestedTrack;
  }
  saveCurrentTrack();
  requestedTrack = 0;
  stepsRemaining = 0;
  currentMoveIsRelative = false;
  currentDirection = MotorDirection::Idle;
}

void onIndexPulse()
{
  if (state != TurntableState::Turning || stepsRemaining == 0)
  {
    return;
  }
  if (currentTrack >= 1 && currentTrack <= TRACK_COUNT)
  {
    currentTrack = advanceTrack(currentTrack, currentDirection);
  }
  if (stepsRemaining > 0)
  {
    --stepsRemaining;
  }
  if (stepsRemaining == 0)
  {
    if (!currentMoveIsRelative && requestedTrack >= 1)
    {
      currentTrack = requestedTrack;
    }
    startLockingSequence();
  }
}

void handleIndexSensor(uint32_t now)
{
  const bool active = digitalRead(PIN_TURNTABLE_SWITCH) == LOW;
  if (active && !lastIndexActive && (now - lastStepMillis) >= STEP_DEBOUNCE_MS)
  {
    lastStepMillis = now;
    onIndexPulse();
  }
  lastIndexActive = active;
}

void handleManualLock(uint32_t now)
{
  if (manualLockMode == ManualLockMode::None)
  {
    return;
  }
  if (manualLockDeadline != 0 && now >= manualLockDeadline)
  {
    lockOutputsOff();
    manualLockDeadline = 0;
  }
  if (manualLockMode == ManualLockMode::Release)
  {
    if (manualLockDeadline == 0 && manualLockSettleDeadline != 0 && now >= manualLockSettleDeadline)
    {
      manualLockMode = ManualLockMode::None;
      manualLockSettleDeadline = 0;
      manualLockDeadline = 0;
    }
  }
  else if (manualLockMode == ManualLockMode::Engage)
  {
    if (manualLockDeadline == 0)
    {
      manualLockMode = ManualLockMode::None;
    }
  }
}

void processPendingCommand()
{
  if (!pendingCommand.active || state != TurntableState::Idle)
  {
    return;
  }
  MoveCommand command = pendingCommand;
  pendingCommand.active = false;
  if (command.steps == 0)
  {
    return;
  }
  currentMoveIsRelative = command.isRelative;
  requestedTrack = command.targetTrack;
  stepsRemaining = command.steps;
  currentDirection = command.direction;
  state = TurntableState::Unlocking;
  setLockRelease();
}

void queueMove(const MoveCommand &command)
{
  if (!command.active || command.steps == 0 || command.direction == MotorDirection::Idle)
  {
    return;
  }
  if (state == TurntableState::Idle)
  {
    pendingCommand = command;
    processPendingCommand();
    return;
  }
  pendingCommand = command;
  pendingCommand.active = true;
}

void requestRelative(uint8_t steps, MotorDirection direction)
{
  if (steps == 0)
  {
    return;
  }
  MoveCommand command;
  command.active = true;
  command.isRelative = true;
  command.steps = steps;
  command.direction = direction;
  command.targetTrack = 0;
  if (currentTrack < 1 || currentTrack > TRACK_COUNT)
  {
    currentTrack = (direction == MotorDirection::Clockwise) ? 1 : TRACK_COUNT;
  }
  queueMove(command);
}

void requestAbsolute(uint8_t targetTrack, MotorDirection preferredDirection)
{
  if (targetTrack < 1 || targetTrack > TRACK_COUNT)
  {
    return;
  }
  if (currentTrack < 1 || currentTrack > TRACK_COUNT)
  {
    return;
  }
  const uint8_t steps = computeSteps(currentTrack, targetTrack, preferredDirection);
  if (steps == 0)
  {
    if (currentTrack != targetTrack)
    {
      return;
    }
    saveCurrentTrack();
    return;
  }
  MoveCommand command;
  command.active = true;
  command.isRelative = false;
  command.steps = steps;
  command.targetTrack = targetTrack;
  command.direction = preferredDirection;
  queueMove(command);
}

void requestTurn180(MotorDirection direction)
{
  if (direction == MotorDirection::Idle)
  {
    return;
  }
  if (currentTrack < 1 || currentTrack > TRACK_COUNT)
  {
    currentTrack = (direction == MotorDirection::Clockwise) ? 1 : TRACK_COUNT;
  }
  MoveCommand command;
  command.active = true;
  command.isRelative = true;
  command.steps = TRACK_COUNT / 2;
  command.direction = direction;
  if (currentTrack >= 1 && currentTrack <= TRACK_COUNT)
  {
    int16_t offset = (direction == MotorDirection::Clockwise) ? (TRACK_COUNT / 2) : -(TRACK_COUNT / 2);
    int16_t track = static_cast<int16_t>(currentTrack) + offset;
    while (track < 1)
    {
      track += TRACK_COUNT;
    }
    while (track > TRACK_COUNT)
    {
      track -= TRACK_COUNT;
    }
    command.targetTrack = static_cast<uint8_t>(track);
  }
  queueMove(command);
}

void handleDccCommand(uint16_t address, uint8_t direction)
{
  for (const auto &entry : accessoryTable)
  {
    if (entry.address != address)
    {
      continue;
    }
    if (entry.address == BASE_ACCESSORY_ADDRESS)
    {
      const MotorDirection turnDirection = (direction == 0) ? MotorDirection::Clockwise : MotorDirection::CounterClockwise;
      requestTurn180(turnDirection);
    }
    else
    {
      const bool isGreen = (direction != 0);
      const uint8_t target = isGreen ? entry.greenTrack : entry.redTrack;
      const MotorDirection preferred = isGreen ? MotorDirection::Clockwise : MotorDirection::CounterClockwise;
      requestAbsolute(target, preferred);
    }
    return;
  }
}

} // namespace

void handleLeftButtonAction(uint32_t pressDuration)
{
  if (pressDuration >= BUTTON_LONG_PRESS_MS)
  {
    requestManualLockEngage();
  }
  else if (pressDuration >= BUTTON_MEDIUM_PRESS_MS)
  {
    requestTurn180(MotorDirection::CounterClockwise);
  }
  else
  {
    requestRelative(1, MotorDirection::CounterClockwise);
  }
}

void handleRightButtonAction(uint32_t pressDuration)
{
  if (pressDuration >= BUTTON_LONG_PRESS_MS)
  {
    requestManualLockRelease();
  }
  else if (pressDuration >= BUTTON_MEDIUM_PRESS_MS)
  {
    requestTurn180(MotorDirection::Clockwise);
  }
  else
  {
    requestRelative(1, MotorDirection::Clockwise);
  }
}

void notifyDccAccTurnoutOutput(uint16_t address, uint8_t direction, uint8_t outputPower)
{
  if (outputPower == 0)
  {
    return;
  }
  handleDccCommand(address, direction);
}

void configurePins()
{
  pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);
  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(PIN_TURNTABLE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LOCK_L1, OUTPUT);
  pinMode(PIN_LOCK_L2, OUTPUT);
  pinMode(PIN_MOTOR_M1, OUTPUT);
  pinMode(PIN_MOTOR_M2, OUTPUT);
  pinMode(PIN_DCC_INPUT, INPUT);

  digitalWrite(PIN_LOCK_L1, LOW);
  digitalWrite(PIN_LOCK_L2, LOW);
  digitalWrite(PIN_MOTOR_M1, LOW);
  digitalWrite(PIN_MOTOR_M2, LOW);

#if defined(ESP8266)
  analogWriteFreq(MOTOR_PWM_FREQUENCY);
  analogWriteRange((1U << MOTOR_PWM_RESOLUTION) - 1U);
  analogWrite(PIN_MOTOR_M1, 0);
  analogWrite(PIN_MOTOR_M2, 0);
#else
  ledcSetup(MOTOR_PWM_CHANNEL_CW, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
  ledcSetup(MOTOR_PWM_CHANNEL_CCW, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
  ledcAttachPin(PIN_MOTOR_M1, MOTOR_PWM_CHANNEL_CW);
  ledcAttachPin(PIN_MOTOR_M2, MOTOR_PWM_CHANNEL_CCW);
#endif
}

void loadCurrentTrack()
{
  currentTrack = EEPROM.read(EEPROM_ADDRESS_CURRENT_TRACK);
  if (currentTrack < 1 || currentTrack > TRACK_COUNT)
  {
    currentTrack = 1;
  }
}

void setup()
{
  Serial.begin(115200);
  initialiseAccessoryTable();
  configurePins();

  EEPROM.begin(32);
  loadCurrentTrack();

  buttonLeft.setDebounceTime(35);
  buttonRight.setDebounceTime(35);

  Dcc.pin(PIN_DCC_INPUT, true);
  Dcc.init(MAN_ID_DIY, 1, FLAGS_OUTPUT_ADDRESS_MODE | FLAGS_DCC_ACCESSORY_DECODER, 0);
}

void handleButtons()
{
  buttonLeft.loop();
  buttonRight.loop();

  const uint32_t now = millis();

  if (buttonLeft.isPressed())
  {
    leftButtonHeld = true;
    leftPressedAt = now;
    leftSuppressed = false;
    if (rightButtonHeld)
    {
      comboActive = true;
      comboStartedAt = (leftPressedAt > rightPressedAt) ? leftPressedAt : rightPressedAt;
    }
  }

  if (buttonRight.isPressed())
  {
    rightButtonHeld = true;
    rightPressedAt = now;
    rightSuppressed = false;
    if (leftButtonHeld)
    {
      comboActive = true;
      comboStartedAt = (leftPressedAt > rightPressedAt) ? leftPressedAt : rightPressedAt;
    }
  }

  if (leftButtonHeld && rightButtonHeld)
  {
    if (!comboActive)
    {
      comboActive = true;
      comboStartedAt = (leftPressedAt > rightPressedAt) ? leftPressedAt : rightPressedAt;
    }
    if (!comboTriggered && (now - comboStartedAt) >= BUTTON_COMBO_HOLD_MS)
    {
      if (state == TurntableState::Idle && manualLockMode == ManualLockMode::None)
      {
        storeCurrentAsTrackOne();
      }
      comboTriggered = true;
      leftSuppressed = true;
      rightSuppressed = true;
    }
  }
  else
  {
    comboActive = false;
    comboStartedAt = 0;
  }

  if (buttonLeft.isReleased())
  {
    const uint32_t duration = now - leftPressedAt;
    leftButtonHeld = false;
    if (!leftSuppressed && !comboTriggered)
    {
      handleLeftButtonAction(duration);
    }
    leftSuppressed = false;
    if (!leftButtonHeld && !rightButtonHeld)
    {
      comboTriggered = false;
    }
  }

  if (buttonRight.isReleased())
  {
    const uint32_t duration = now - rightPressedAt;
    rightButtonHeld = false;
    if (!rightSuppressed && !comboTriggered)
    {
      handleRightButtonAction(duration);
    }
    rightSuppressed = false;
    if (!leftButtonHeld && !rightButtonHeld)
    {
      comboTriggered = false;
    }
  }

  if (!leftButtonHeld && !rightButtonHeld && !comboTriggered)
  {
    comboStartedAt = 0;
  }
}

void updateTurntable()
{
  const uint32_t now = millis();
  handleManualLock(now);
  switch (state)
  {
    case TurntableState::Idle:
      if (manualLockMode == ManualLockMode::None)
      {
        processPendingCommand();
      }
      break;
    case TurntableState::Unlocking:
      if (now >= lockDeadline)
      {
        lockOutputsOff();
        state = TurntableState::WaitingForUnlock;
        settleDeadline = now + UNLOCK_SETTLE_DURATION_MS;
      }
      break;
    case TurntableState::WaitingForUnlock:
      if (now >= settleDeadline)
      {
        driveMotor(currentDirection);
        state = TurntableState::Turning;
      }
      break;
    case TurntableState::Turning:
      handleIndexSensor(now);
      break;
    case TurntableState::Locking:
      if (now >= lockDeadline)
      {
        lockOutputsOff();
        state = TurntableState::LockSettling;
        settleDeadline = now + LOCK_SETTLE_DURATION_MS;
      }
      break;
    case TurntableState::LockSettling:
      if (now >= settleDeadline)
      {
        finalizeMovement();
        state = TurntableState::Idle;
        processPendingCommand();
      }
      break;
  }
}

void loop()
{
  Dcc.process();
  handleButtons();
  updateTurntable();
}
