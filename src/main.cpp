// SEGMENT A START — Includes And Global State
#include <M5Cardputer.h>
#include <USB.h>

#include "bluetooth.h"
#include "display.h"
#include "usbHid.h"

DeviceMode currentMode = DeviceMode::Keyboard;
bool usbMode = true;
bool lastBluetoothStatus = false;
int lastBatteryLevel = -1;
unsigned long lastBatteryRefreshMs = 0;

DeviceMode nextMode(DeviceMode mode) {
    switch (mode) {
        case DeviceMode::Keyboard:
            return DeviceMode::Mouse;
        case DeviceMode::Mouse:
            return DeviceMode::Hotkey;
        case DeviceMode::Hotkey:
        default:
            return DeviceMode::Keyboard;
    }
}

bool isMouseMode(DeviceMode mode) {
    return mode == DeviceMode::Mouse;
}
// SEGMENT A END — Includes And Global State

// SEGMENT B START — Transport Selection
void selectMode() {
    bool lastMode = !usbMode;

    while (true) {
        M5Cardputer.update();

        if (lastMode != usbMode) {
            displaySelectionScreen(usbMode);
            lastMode = usbMode;
        }

        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            if (M5Cardputer.Keyboard.isKeyPressed('.') || M5Cardputer.Keyboard.isKeyPressed(';')) {
                usbMode = !usbMode;
            }

            if (status.enter) {
                break;
            }
        }

        delay(10);
    }
}
// SEGMENT B END — Transport Selection

// SEGMENT C START — Setup And Main Loop
void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);

    setupDisplay();
    displayWelcomeScreen();
    selectMode();

    if (usbMode) {
        USB.begin();
    } else {
        initBluetooth();
    }

    lastBluetoothStatus = getBluetoothStatus();
    lastBatteryLevel = M5Cardputer.Power.getBatteryLevel();
    lastBatteryRefreshMs = millis();

    displayMainScreen(usbMode, currentMode, lastBluetoothStatus, lastBatteryLevel);
}

void loop() {
    M5Cardputer.update();

    const bool bluetoothStatus = getBluetoothStatus();

    if (lastBluetoothStatus != bluetoothStatus) {
        modeIndicator(usbMode, bluetoothStatus);
        lastBluetoothStatus = bluetoothStatus;
    }

    if (M5Cardputer.BtnA.wasPressed()) {
        currentMode = nextMode(currentMode);
        drawModeCards(currentMode);
    }

    const unsigned long now = millis();
    if ((now - lastBatteryRefreshMs) >= 1000UL) {
        const int batteryLevel = M5Cardputer.Power.getBatteryLevel();
        if (batteryLevel != lastBatteryLevel) {
            drawBatteryLevel(batteryLevel);
            lastBatteryLevel = batteryLevel;
        }
        lastBatteryRefreshMs = now;
    }

    if (usbMode) {
        handleUsbMode(currentMode);
    } else {
        handleBluetoothMode(currentMode);
    }
}
// SEGMENT C END — Setup And Main Loop