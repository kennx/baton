# Baton Refactor Design Spec

## 1. Context & Problem Statement
The M5StickS3 hardware features an IR transmitter but **no built-in IR receiver**. The original Baton project attempted to implement IR learning (SCAN mode) and brute-forcing (saving received signals) by reading from GPIO 42, which naturally failed because there is no receiver. The user wants to pivot the project into a universal IR remote using a purely offline, transmission-only architecture.

## 2. Core Architecture Changes
- **Remove IR Receiver Code**: Delete all RMT RX code from `IRController.cpp` and `IRController.h` (e.g., `setupRmtRx`, `enableReceive`, `decodeSignal`, `hasReceived`). 
- **Remove Learning Mode**: Delete `ScanMode.cpp` and `ScanMode.h`.
- **Refactor Brute-Force into Match Mode**: `BruteForceMode` will become `MatchMode`. It will send test codes (like Power Toggle) sequentially from an offline database. When a test code successfully operates the target appliance, the user saves that *Device Profile* instead of a single signal.
- **Refactor Storage**: `SignalStorage` will be renamed/refactored into `DeviceStorage`. Instead of saving arbitrary decoded signals, it will save the selected `Protocol` and `Address` for a specific appliance (e.g., Samsung TV).

## 3. Data Model (Offline IR Database)
The system will contain a hardcoded (static) C++ database of common appliance IR protocols and their command mappings.
For example, for a "Samsung TV":
- Protocol: `SAMSUNG`
- Address: `0x0707`
- Commands:
  - Power: `0x02`
  - Vol+: `0x07`
  - Vol-: `0x0B`
  - Ch+: `0x12`
  - Ch-: `0x10`
  - Mute: `0x0F`

*Note: The exact command hex values will be derived from standard open-source IR databases (e.g., LIRC or Flipper IRDB conventions).*

## 4. UI Interaction & Key Mapping
The M5StickS3 has a 135x240 screen and two usable buttons (BtnA on the front, BtnB on the side).

### 4.1 Home Screen
- Displays a list of saved `Device Profiles` and an option to "Add New Device" (Match Mode).
- **Short Press BtnA**: Enter the selected mode/device.
- **Long Press BtnA**: Cycle through the menu options.

### 4.2 Match Mode (Add Device)
- The user selects the brand (e.g., Samsung, LG, Sony).
- The device blasts the known Power code for that brand.
- If the TV turns off, the user presses a button to confirm and save the profile.

### 4.3 Control Mode
When a saved device is selected from the Home Screen:
- The screen shows the active command (e.g., `[VOL +]`).
- **Short Press BtnA**: Transmit the displayed command via IR.
- **Long Press BtnA**: Cycle to the next command (`VOL -`, `CH +`, `CH -`, `POWER`, etc.).
- **Short Press BtnB**: Go back to the Home Screen.

## 5. Implementation Steps
1. Clean up `IRController` and remove `ScanMode`.
2. Define the static offline database (structs mapping Brands -> Protocols -> Commands).
3. Refactor `SignalStorage` to `DeviceStorage`.
4. Rewrite the matching flow (replace BruteForceMode with `MatchMode`).
5. Rewrite `ControlMode` to support the new long-press UI rotation logic.

## 6. Verification
- Compile code successfully using PlatformIO.
- Verify that standard IR commands are emitted from the IR LED on GPIO 46.
- Verify that long press vs short press logic on BtnA and BtnB correctly navigates the UI and transmits signals.
