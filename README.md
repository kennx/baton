# IR Scanner for M5StickS3

A universal infrared remote learning and replay system for the M5StickS3. Scan IR signals from your existing remotes, save them, and replay them to control your devices.

## Hardware

- [M5StickS3](https://docs.m5stack.com/en/core/M5StickS3) (ESP32-S3, 8MB Flash + 8MB PSRAM)
- Built-in IR transmitter (GPIO46) and receiver (GPIO42)
- 135×240 ST7789 display
- Single button (BtnA) operation

## Build & Flash

```bash
# Clone and build
pio run -e m5stick-s3

# Upload to device
pio run -e m5stick-s3 --target upload

# Monitor serial output
pio device monitor -b 115200
```

## How to Use

All interaction uses a single button with two gestures:

| Gesture | Action |
|---------|--------|
| **Short press** BtnA | Move selection down |
| **Long press** BtnA (>500ms) | Confirm / enter selected item |

---

### Main Menu

Power on to enter the main menu. Four options are shown:

| Menu | Purpose |
|------|---------|
| **Scan** | Learn IR signals from your remotes |
| **Ctrl** | Browse saved signals and transmit them |
| **Mgmt** | Rename, categorize, or delete saved signals |
| **Set** | Adjust repeat count, brightness, buzzer, etc. |

Short press to cycle through menus. Long press to enter the selected mode.

---

### Scan Mode

Automatically listens for infrared signals.

- Point any IR remote at the M5StickS3 and press a button.
- The signal is decoded, deduplicated, and saved automatically.
- Status messages appear for 2 seconds:
  - `OK New signal saved` — first time seeing this signal
  - `DUP - skip` — already saved, not stored again
  - `FULL! Del old sigs` — storage limit (50 signals) reached
- A short beep confirms a new save (if buzzer is enabled).
- Long press BtnA to return to the main menu.

---

### Ctrl Mode

Browse your saved signals and transmit them.

- Short press BtnA to scroll through the signal list.
- The last item in the list is `-- FILTER --`; select it to filter by category (All / TV / AC / FAN / STB / Uncat).
- Long press BtnA on a signal to **transmit** it.
- A brief `Sending...` screen appears while the IR LED fires.
- The signal is sent with the configured repeat count (default 2).
- Long press BtnA on the first item to return to the main menu.

---

### Mgmt Mode

Manage your saved signals.

1. Short press to select a signal, then long press to open the action menu.
2. Choose an action:

| Action | Description |
|--------|-------------|
| **Rename** | Pick a category (TV / AC / FAN / STB / Custom) and a preset name. The signal is renamed to `Category-Name`, e.g. `TV-Power`. |
| **Categorize** | Move the signal to a different folder without renaming it. |
| **Delete** | Shows a confirmation popup. Select `Confirm` and long press to permanently remove the signal. |
| **Raw data** | View technical details (protocol, address, command, raw length). Long press to go back. |
| **Back** | Return to the signal list. |

---

### Set Mode

Adjust system settings.

| Setting | Default | Behavior |
|---------|---------|----------|
| **Clear all** | — | Deletes all saved signals after confirmation. |
| **Repeat** | 2 | IR transmit repeat count (cycles 1→2→3→4→5→1). |
| **Brightness** | 80% | Screen brightness (cycles 20→40→60→80→100→20). Changes apply immediately. |
| **Buzzer** | ON | Enable / disable the beep sound on new signal save. |
| **About** | — | Shows firmware version, signal count, and storage usage. |

Long press on any setting to adjust it. Long press `About` to view; long press again to return.

---

## Supported IR Protocols

The system can **decode** and **transmit** signals for these protocols:

- NEC / NECX / NEC42
- Sony
- Samsung / Samsung36 / SamsungAC
- LG / LG2
- RC5 / RC5X
- RC6 / RC-MM
- Panasonic / PanasonicAC
- Sharp

Signals from unknown protocols are captured as raw timing arrays and replayed exactly as received.

## Storage

- Up to **50 signals** can be saved.
- Signals are stored in LittleFS (on-board Flash).
- Each signal preserves its raw timing data for exact replay.
- Data persists across power cycles.

## Notes

- **Line of sight** is required between the M5StickS3 and the target device during transmission.
- The IR receiver may pick up ambient IR noise; deduplication prevents duplicate saves from the same button press.
- If a signal fails to control your device, try increasing the **Repeat** count in settings.
- The built-in speaker amp is disabled during IR reception to avoid interference.
