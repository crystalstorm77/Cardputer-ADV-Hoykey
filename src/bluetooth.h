// SEGMENT A START — Bluetooth Includes And Globals
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

// M5Cardputer defines these as macros in Keyboard_def.h.
// ESP32 BLE Keyboard defines constants with the same names in BleKeyboard.h.
// Undef them here before including BleKeyboard.h so both libraries can coexist.
#ifdef KEY_LEFT_CTRL
#undef KEY_LEFT_CTRL
#endif

#ifdef KEY_LEFT_SHIFT
#undef KEY_LEFT_SHIFT
#endif

#ifdef KEY_LEFT_ALT
#undef KEY_LEFT_ALT
#endif

#ifdef KEY_BACKSPACE
#undef KEY_BACKSPACE
#endif

#ifdef KEY_TAB
#undef KEY_TAB
#endif

#include <BleKeyboard.h>
#include "display.h"

extern BleKeyboard bleKeyboard;
extern bool bluetoothIsConnected;
// SEGMENT A END — Bluetooth Includes And Globals

// SEGMENT B START — Bluetooth Function Declarations
void initBluetooth();
void deinitBluetooth();
bool getBluetoothStatus();

void bluetoothMouse();
void bluetoothKeyboard();
void bluetoothHotkey();
void sendEmptyReports();
void handleBluetoothMode(DeviceMode currentMode);

#endif // BLUETOOTH_H
// SEGMENT B END — Bluetooth Function Declarations