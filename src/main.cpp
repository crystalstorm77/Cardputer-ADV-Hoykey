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