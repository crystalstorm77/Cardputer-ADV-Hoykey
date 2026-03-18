// SEGMENT A START — Hotkey Includes And Constants
#include "hotkeys.h"

namespace {
constexpr uint8_t HID_KEY_A = 0x04;
constexpr uint8_t HID_KEY_3 = 0x20;
constexpr uint8_t HID_KEY_4 = 0x21;
constexpr uint8_t HID_KEY_PAUSE = 0x48;
constexpr uint8_t HID_KEY_F13 = 0x68;
constexpr uint8_t HID_KEY_F14 = 0x69;

constexpr uint8_t MODIFIER_LEFT_CTRL = 0x01;
constexpr uint8_t MODIFIER_LEFT_ALT = 0x04;

bool statusContainsHidKey(const Keyboard_Class::KeysState& status, uint8_t hidKey) {
    for (auto key : status.hid_keys) {
        if (key == hidKey) {
            return true;
        }
    }
    return false;
}

void clearReport(HotkeyReport& report) {
    report.modifiers = 0;
    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = 0;
    }
}

void setSingleKeyReport(HotkeyReport& report, uint8_t hidKey) {
    clearReport(report);
    report.keys[0] = hidKey;
}

void setModifierComboReport(HotkeyReport& report, uint8_t modifiers, uint8_t hidKey) {
    clearReport(report);
    report.modifiers = modifiers;
    report.keys[0] = hidKey;
}
}  // namespace
// SEGMENT A END — Hotkey Includes And Constants

// SEGMENT B START — Hotkey Mapping Logic
bool buildHotkeyReport(const Keyboard_Class::KeysState& status, HotkeyReport& report) {
    clearReport(report);

    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        setSingleKeyReport(report, HID_KEY_PAUSE);
        return true;
    }

    if (statusContainsHidKey(status, HID_KEY_A)) {
        setModifierComboReport(report, MODIFIER_LEFT_CTRL | MODIFIER_LEFT_ALT, HID_KEY_A);
        return true;
    }

    if (statusContainsHidKey(status, HID_KEY_3)) {
        setSingleKeyReport(report, HID_KEY_F13);
        return true;
    }

    if (statusContainsHidKey(status, HID_KEY_4)) {
        setSingleKeyReport(report, HID_KEY_F14);
        return true;
    }

    return false;
}
// SEGMENT B END — Hotkey Mapping Logic