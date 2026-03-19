// SEGMENT A START — USB HID Includes And Global State
#include <M5Cardputer.h>

#undef KEY_BACKSPACE
#undef KEY_TAB
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"

#include "hotkeys.h"
#include "usbHid.h"

USBHIDMouse mouse;
USBHIDKeyboard keyboard;

namespace {
bool keyboardInited = false;
bool mouseInited = false;

void ensureKeyboardReady() {
    if (!keyboardInited) {
        keyboard.begin();
        keyboardInited = true;
    }
}

void ensureMouseReady() {
    if (!mouseInited) {
        mouse.begin();
        mouseInited = true;
    }
}

void sendUsbHotkeyReport(const HotkeyReport& hotkeyReport) {
    KeyReport report = {0};
    report.modifiers = hotkeyReport.modifiers;

    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = hotkeyReport.keys[i];
    }

    keyboard.sendReport(&report);
}

void sendUsbKeyboardModeReport(const KeyboardModeReport& keyboardModeReport) {
    KeyReport report = {0};
    report.modifiers = keyboardModeReport.modifiers;

    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = keyboardModeReport.keys[i];
    }

    keyboard.sendReport(&report);
}
}  // namespace

void initUsbHid() {
    ensureKeyboardReady();
    ensureMouseReady();
}
// SEGMENT A END — USB HID Includes And Global State

// SEGMENT B START — USB HID Input Modes
void handleUsbMode(DeviceMode currentMode) {
    switch (currentMode) {
        case DeviceMode::Mouse:
            usbMouse();
            break;
        case DeviceMode::Hotkey:
            usbHotkey();
            break;
        case DeviceMode::Keyboard:
        default:
            usbKeyboard();
            break;
    }
    delay(5);
}

void usbMouse() {
    mouse.begin();
    int moveX = 0;
    int moveY = 0;

    if (M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

        if (M5Cardputer.Keyboard.isKeyPressed('/')) {
            moveX = 1;
        }

        if (M5Cardputer.Keyboard.isKeyPressed(',')) {
            moveX = -1;
        }

        if (M5Cardputer.Keyboard.isKeyPressed(';')) {
            moveY = -1;
        }

        if (M5Cardputer.Keyboard.isKeyPressed('.')) {
            moveY = 1;
        }

        if (status.enter) {
            mouse.press(MOUSE_BUTTON_LEFT);
        } else if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
            mouse.press(MOUSE_BUTTON_RIGHT);
        }

        mouse.move(moveX, moveY);
    } else {
        mouse.release(MOUSE_BUTTON_LEFT);
        mouse.release(MOUSE_BUTTON_RIGHT);
    }
}

void usbKeyboard() {
    ensureKeyboardReady();

    if (!M5Cardputer.Keyboard.isChange()) {
        return;
    }

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    KeyboardModeReport report = {};
    buildKeyboardModeReport(status, report);

    if (keyboardModeReportHasOutput(report)) {
        sendUsbKeyboardModeReport(report);
    } else {
        keyboard.releaseAll();
    }
}

void usbHotkey() {
    ensureKeyboardReady();

    if (!M5Cardputer.Keyboard.isChange()) {
        return;
    }

    HotkeyReport hotkeyReport = {};
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (buildHotkeyReport(status, hotkeyReport)) {
        sendUsbHotkeyReport(hotkeyReport);
    } else {
        keyboard.releaseAll();
    }
}
// SEGMENT B END — USB HID Input Modes