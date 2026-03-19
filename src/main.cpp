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

const char* modeName(DeviceMode mode) {
    switch (mode) {
        case DeviceMode::Keyboard:
            return "Keyboard";
        case DeviceMode::Mouse:
            return "Mouse";
        case DeviceMode::Hotkey:
            return "Hotkey";
        default:
            return "Unknown";
    }
}

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

void printCharDebugValue(const char* label, char value) {
    const uint8_t rawValue = static_cast<uint8_t>(value);

    Serial.printf("%s=%u", label, rawValue);
    if (rawValue >= 32 && rawValue <= 126) {
        Serial.printf("('%c')", value);
    }
}

void printUint8Vector(const char* label, const std::vector<uint8_t>& values) {
    Serial.printf("%s[%u]=", label, static_cast<unsigned>(values.size()));
    if (values.empty()) {
        Serial.println("[]");
        return;
    }

    Serial.print("[");
    for (size_t i = 0; i < values.size(); ++i) {
        Serial.print(values[i]);
        if ((values[i] >= 32) && (values[i] <= 126)) {
            Serial.printf("('%c')", static_cast<char>(values[i]));
        }
        if (i + 1 < values.size()) {
            Serial.print(", ");
        }
    }
    Serial.println("]");
}

void printWordVector(const std::vector<char>& values) {
    Serial.printf("word[%u]=", static_cast<unsigned>(values.size()));
    if (values.empty()) {
        Serial.println("[]");
        return;
    }

    Serial.print("[");
    for (size_t i = 0; i < values.size(); ++i) {
        const uint8_t rawValue = static_cast<uint8_t>(values[i]);
        Serial.print(rawValue);
        if ((rawValue >= 32) && (rawValue <= 126)) {
            Serial.printf("('%c')", values[i]);
        }
        if (i + 1 < values.size()) {
            Serial.print(", ");
        }
    }
    Serial.println("]");
}

void printPhysicalKeyList() {
    const auto& keyList = M5Cardputer.Keyboard.keyList();
    Serial.printf("keyList[%u]\n", static_cast<unsigned>(keyList.size()));

    for (size_t i = 0; i < keyList.size(); ++i) {
        const auto& keyPos = keyList[i];
        const auto keyValue = M5Cardputer.Keyboard.getKeyValue(keyPos);

        Serial.printf("  #%u pos=(%d,%d) ", static_cast<unsigned>(i), keyPos.x, keyPos.y);
        printCharDebugValue("first", keyValue.value_first);
        Serial.print(" ");
        printCharDebugValue("second", keyValue.value_second);
        Serial.println();
    }
}

void printKeyboardDebugSnapshot() {
    auto& status = M5Cardputer.Keyboard.keysState();

    Serial.println();
    Serial.println("=== CARDPUTER KEY DEBUG ===");
    Serial.printf("mode=%s usbMode=%d pressed=%d changed=%d capsLocked=%d\n",
                  modeName(currentMode),
                  usbMode ? 1 : 0,
                  M5Cardputer.Keyboard.isPressed() ? 1 : 0,
                  M5Cardputer.Keyboard.isChange() ? 1 : 0,
                  M5Cardputer.Keyboard.capslocked() ? 1 : 0);

    Serial.printf("flags fn=%d shift=%d ctrl=%d opt=%d alt=%d del=%d enter=%d space=%d modifiers=0x%02X\n",
                  status.fn ? 1 : 0,
                  status.shift ? 1 : 0,
                  status.ctrl ? 1 : 0,
                  status.opt ? 1 : 0,
                  status.alt ? 1 : 0,
                  status.del ? 1 : 0,
                  status.enter ? 1 : 0,
                  status.space ? 1 : 0,
                  status.modifiers);

    printWordVector(status.word);
    printUint8Vector("hid_keys", status.hid_keys);
    printUint8Vector("modifier_keys", status.modifier_keys);
    printPhysicalKeyList();

    Serial.println("===========================");
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

    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("Cardputer keyboard debug active.");
    Serial.println("Open PlatformIO Serial Monitor at 115200 baud.");
    Serial.println("Stay in Keyboard mode and test Fn by itself and with the orange-labelled keys.");

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

    if (currentMode == DeviceMode::Keyboard && M5Cardputer.Keyboard.isChange()) {
        printKeyboardDebugSnapshot();
    }

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