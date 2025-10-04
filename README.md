# Stand-alone Kato 20-283 Turntable Controller

This repository contains an ESP32 (ESP12F) firmware project that drives a Kato 20-283 turntable directly without the DCCNext daughter board. The code implements the Fleischmann 6915 command set so the bridge can be controlled from software such as iTrain or from a command station such as the ESU ECoS.

## Features

* Native ESP32 accessory decoder using the [`NmraDcc`](https://github.com/mrrwa/NmraDcc) library.
* Accessory addresses **500 – 518** (red/green outputs) select tracks 1 – 36. Address **500** performs a 180° rotation (red = clockwise, green = counter-clockwise).
* Hardware lock sequencing that pulses the lock coils, waits for the bridge to release, turns the bridge, and re-locks the bridge when in position.
* Manual control via two buttons on GPIO 9 (counter-clockwise) and GPIO 10 (clockwise), including 180° turns, lock service pulses, and home-track storage.
* Persistent storage of the last known bridge track in the ESP32 EEPROM.
* Designed for use with an external auto-reverser: no bridge polarity switching is performed by this firmware.

## Hardware mapping

| Function             | GPIO |
|----------------------|:----:|
| Button – Left (CCW)  |  9   |
| Button – Right (CW)  | 10   |
| Turntable lock L1    | 16   |
| Turntable lock L2    |  2   |
| Turntable index      |  4   |
| DCC input            |  5   |
| Motor output M1      | 12   |
| Motor output M2      | 14   |

The DCC input should feed GPIO 5 through an opto-isolator (for example a 6N137 wired as in the reference circuit supplied with the hardware).

## PlatformIO project layout

```
.
├── .vscode/                # VS Code workspace recommendations
├── include/                # (empty) header folder for future use
├── lib/                    # (empty) private libraries folder
├── src/
│   └── main.cpp            # Firmware entry point
└── platformio.ini          # Build configuration for ESP32/Arduino
```

Both `include/` and `lib/` are created automatically by PlatformIO when you first build the project. They remain empty in this repository so you can immediately start adding headers or custom libraries if required.

## Building and flashing with VS Code + PlatformIO

1. Install [Visual Studio Code](https://code.visualstudio.com/) and add the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode). Opening this repository will prompt you to install the recommended extension.
2. Open the project folder (`File → Open Folder…`). PlatformIO will detect `platformio.ini` and prepare the ESP32 toolchain.
3. Connect your ESP32/ESP12F board, then in the PlatformIO toolbar select **ESP32 Dev Module** (defined in `platformio.ini`).
4. Build the firmware with **PlatformIO: Build** (`Ctrl/Cmd + Alt + B`) and upload it using **PlatformIO: Upload** (`Ctrl/Cmd + Alt + U`).
5. The accessory decoder address range is hard-coded; no CV programming is required after flashing.

The firmware depends on the following libraries, which are fetched automatically by PlatformIO:

* [`NmraDcc`](https://github.com/mrrwa/NmraDcc)
* [`ezButton`](https://github.com/ArduinoGetStarted/button)

## Manual controls

The two physical buttons replicate the original three-button feature set using short, medium, and long presses. Hold times are measured from the moment the button is depressed; releasing the button executes the action.

| Gesture | Action |
|---------|--------|
| Left short press (< 0.7 s) | Step 1 track counter-clockwise |
| Right short press (< 0.7 s) | Step 1 track clockwise |
| Left medium press (0.7 – 2 s) | Queue a 180° rotation counter-clockwise |
| Right medium press (0.7 – 2 s) | Queue a 180° rotation clockwise |
| Left long press (≥ 2 s) | Pulse the lock closed (manual engage) |
| Right long press (≥ 2 s) | Pulse the lock open (manual release) |
| Hold both buttons (≥ 2 s) | Store the current bridge position as track 1 (home/track 0 reference) |

Manual lock pulses and home storage only run while the bridge controller is idle. The combined-button gesture mirrors the original "set track 0" function from the three-button controller.

## Operating notes

* The bridge position is stored in EEPROM. If the stored track is not valid after flashing, manually align the bridge to track 1 and use the **hold both buttons** gesture to store the home position.
* Accessory address 500 (red) rotates the bridge 180° clockwise; address 500 (green) rotates 180° counter-clockwise.
* Addresses 501 – 518 move to the programmed tracks:
  * Red output → track 1 – 18 (counter-clockwise direction)
  * Green output → track 19 – 36 (clockwise direction)
* The firmware queues a new DCC command when the bridge is already turning; the queued command starts automatically once the current movement finishes.

