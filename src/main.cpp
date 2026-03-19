// SEGMENT A START — Includes And Global State
#include <M5Cardputer.h>
#include <USB.h>
#include <cstdio>

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

void formatDebugChar(char value, char* output, size_t outputSize) {
    const uint8_t rawValue = static_cast<uint8_t>(value);

    if (rawValue == 0) {
        std::snprintf(output, outputSize, "0");
    } else if (rawValue >= 32 && rawValue <= 126) {
        std::snprintf(output, outputSize, "%u:%c", rawValue, value);
    } else {
        std::snprintf(output, outputSize, "%u", rawValue);
    }
}

void buildPhysicalKeyLine(size_t index, char* output, size_t outputSize) {
    const auto& keyList = M5Cardputer.Keyboard.keyList();

    if (index >= keyList.size()) {
        std::snprintf(output, outputSize, "p%u: -", static_cast<unsigned>(index));
        return;
    }

    const auto& keyPos = keyList[index];
    const auto keyValue = M5Cardputer.Keyboard.getKeyValue(keyPos);

    char firstValue[16];
    char secondValue[16];
    formatDebugChar(keyValue.value_first, firstValue, sizeof(firstValue));
    formatDebugChar(keyValue.value_second, secondValue, sizeof(secondValue));

    std::snprintf(
        output,
        outputSize,
        "p%u %d,%d f=%s s=%s",
        static_cast<unsigned>(index),
        keyPos.x,
        keyPos.y,
        firstValue,
        secondValue
    );
}

void updateKeyboardDebugOverlay() {
    auto& status = M5Cardputer.Keyboard.keysState();

    const uint8_t hid0 = status.hid_keys.size() > 0 ? status.hid_keys[0] : 0;
    const uint8_t hid1 = status.hid_keys.size() > 1 ? status.hid_keys[1] : 0;
    const uint8_t hid2 = status.hid_keys.size() > 2 ? status.hid_keys[2] : 0;

    char line1[64];
    char line2[64];
    char line3[64];
    char line4[64];
    char line5[64];

    std::snprintf(
        line1,
        sizeof(line1),
        "fn=%d sh=%d ct=%d al=%d dl=%d en=%d",
        status.fn ? 1 : 0,
        status.shift ? 1 : 0,
        status.ctrl ? 1 : 0,
        status.alt ? 1 : 0,
        status.del ? 1 : 0,
        status.enter ? 1 : 0
    );

    std::snprintf(
        line2,
        sizeof(line2),
        "hid=%u,%u,%u mods=%02X",
        hid0,
        hid1,
        hid2,
        status.modifiers
    );

    buildPhysicalKeyLine(0, line3, sizeof(line3));
    buildPhysicalKeyLine(1, line4, sizeof(line4));
    buildPhysicalKeyLine(2, line5, sizeof(line5));

    drawKeyboardDebugOverlay(line1, line2, line3, line4, line5);
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

    if (currentMode == DeviceMode::Keyboard) {
        drawKeyboardDebugOverlay(
            "Press Fn alone first",
            "Then test Fn+, Fn+;",
            "Watch p0 / p1 / p2",
            "Tell me what changes",
            ""
        );
    }
}

void loop() {
    M5Cardputer.update();

    if (currentMode == DeviceMode::Keyboard && M5Cardputer.Keyboard.isChange()) {
        updateKeyboardDebugOverlay();
    }

    const bool bluetoothStatus = getBluetoothStatus();

    if (lastBluetoothStatus != bluetoothStatus) {
        modeIndicator(usbMode, bluetoothStatus);
        lastBluetoothStatus = bluetoothStatus;
    }

    if (M5Cardputer.BtnA.wasPressed()) {
        currentMode = nextMode(currentMode);
        drawModeCards(currentMode);

        if (currentMode == DeviceMode::Keyboard) {
            drawKeyboardDebugOverlay(
                "Press Fn alone first",
                "Then test Fn+, Fn+;",
                "Watch p0 / p1 / p2",
                "Tell me what changes",
                ""
            );
        }
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