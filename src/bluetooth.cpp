// SEGMENT A START — Bluetooth Includes And Global State
#include "bluetooth.h"

#include "hotkeys.h"

BleKeyboard bleKeyboard("M5-Keyboard-Mouse", "M5Stack", 100);
bool bluetoothIsConnected = false;

namespace {
bool bluetoothHadActiveInput = false;

void sendKeyboardReport(uint8_t modifier, const uint8_t keycode[6]) {
    KeyReport report = {0};
    report.modifiers = modifier;

    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = keycode[i];
    }

    bleKeyboard.sendReport(&report);
}
}  // namespace
// SEGMENT A END — Bluetooth Includes And Global State

// SEGMENT B START — Bluetooth Input Modes
bool getBluetoothStatus() {
    bluetoothIsConnected = bleKeyboard.isConnected();
    return bluetoothIsConnected;
}

void bluetoothMouse() {
    // Temporary diagnostic:
    // Bluetooth mouse mode remains disabled while we test keyboard-only
    // reconnect reliability with a different BLE transport.
    sendEmptyReports();
}

void bluetoothKeyboard() {
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    KeyboardModeReport report = {};
    buildKeyboardModeReport(status, report);

    if (keyboardModeReportHasOutput(report)) {
        sendKeyboardReport(report.modifiers, report.keys);
        bluetoothHadActiveInput = true;
    } else if (bluetoothHadActiveInput) {
        sendEmptyReports();
        bluetoothHadActiveInput = false;
    }
}

void bluetoothHotkey() {
    HotkeyReport hotkeyReport = {};
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (buildHotkeyReport(status, hotkeyReport)) {
        sendKeyboardReport(hotkeyReport.modifiers, hotkeyReport.keys);
        bluetoothHadActiveInput = true;
    } else if (bluetoothHadActiveInput) {
        sendEmptyReports();
        bluetoothHadActiveInput = false;
    }
}

void sendEmptyReports() {
    bleKeyboard.releaseAll();
}

void handleBluetoothMode(DeviceMode currentMode) {
    bluetoothIsConnected = bleKeyboard.isConnected();

    if (!bluetoothIsConnected) {
        bluetoothHadActiveInput = false;
        delay(7);
        return;
    }

    if (!M5Cardputer.Keyboard.isPressed()) {
        if (bluetoothHadActiveInput) {
            sendEmptyReports();
            bluetoothHadActiveInput = false;
        }
        delay(7);
        return;
    }

    switch (currentMode) {
        case DeviceMode::Mouse:
            bluetoothMouse();
            break;
        case DeviceMode::Hotkey:
            bluetoothHotkey();
            break;
        case DeviceMode::Keyboard:
        default:
            bluetoothKeyboard();
            break;
    }

    delay(7);
}
// SEGMENT B END — Bluetooth Input Modes

// SEGMENT C START — Bluetooth Setup And Teardown
void initBluetooth() {
    bluetoothHadActiveInput = false;
    bleKeyboard.begin();
}

void deinitBluetooth() {
    bluetoothHadActiveInput = false;
    bleKeyboard.end();
}
// SEGMENT C END — Bluetooth Setup And Teardown