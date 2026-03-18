// SEGMENT A START — Hotkey Includes And Types
#include "hotkeys.h"

constexpr uint8_t HID_KEY_A = 0x04;
constexpr uint8_t HID_KEY_3 = 0x20;
constexpr uint8_t HID_KEY_4 = 0x21;
constexpr uint8_t HID_KEY_PAUSE = 0x48;
constexpr uint8_t HID_KEY_F13 = 0x68;
constexpr uint8_t HID_KEY_F14 = 0x69;

constexpr uint8_t MODIFIER_NONE = 0x00;
constexpr uint8_t MODIFIER_LEFT_CTRL = 0x01;
constexpr uint8_t MODIFIER_LEFT_SHIFT = 0x02;
constexpr uint8_t MODIFIER_LEFT_ALT = 0x04;

enum class HotkeyTriggerType : uint8_t {
    PrintableChar = 0,
    HidKey = 1
};

struct HotkeyBinding {
    HotkeyTriggerType triggerType;
    char printableKey;
    uint8_t triggerHidKey;
    uint8_t modifiers;
    uint8_t outputKeys[6];
};
// SEGMENT A END — Hotkey Includes And Types

// SEGMENT B START — Editable Hotkey Bindings
// Edit this table for most future hotkey changes.
// Rules:
// 1) First matching entry wins.
// 2) PrintableChar is best for keys like space.
// 3) HidKey is best for letters/numbers that should map cleanly regardless of layer handling.
// 4) outputKeys supports up to 6 simultaneous HID keys.
// 5) modifiers uses standard HID modifier bits:
//    Left Ctrl = 0x01, Left Shift = 0x02, Left Alt = 0x04.

const HotkeyBinding kHotkeyBindings[] = {
    // Trigger                                      Modifiers                               Output keys
    {HotkeyTriggerType::PrintableChar, ' ', 0,      MODIFIER_NONE,                          {HID_KEY_PAUSE, 0, 0, 0, 0, 0}}, // Space -> Pause
    {HotkeyTriggerType::HidKey,        0,   HID_KEY_A, MODIFIER_LEFT_CTRL | MODIFIER_LEFT_ALT, {HID_KEY_A, 0, 0, 0, 0, 0}}, // A -> Ctrl+Alt+A
    {HotkeyTriggerType::HidKey,        0,   HID_KEY_3, MODIFIER_NONE,                       {HID_KEY_F13, 0, 0, 0, 0, 0}}, // 3 -> F13
    {HotkeyTriggerType::HidKey,        0,   HID_KEY_4, MODIFIER_NONE,                       {HID_KEY_F14, 0, 0, 0, 0, 0}}, // 4 -> F14
};

constexpr int kHotkeyBindingCount = sizeof(kHotkeyBindings) / sizeof(kHotkeyBindings[0]);
// SEGMENT B END — Editable Hotkey Bindings

// SEGMENT C START — Hotkey Lookup And Report Builder
namespace {
void clearReport(HotkeyReport& report) {
    report.modifiers = 0;
    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = 0;
    }
}

bool statusContainsHidKey(const Keyboard_Class::KeysState& status, uint8_t hidKey) {
    for (auto key : status.hid_keys) {
        if (key == hidKey) {
            return true;
        }
    }
    return false;
}

bool bindingTriggered(const Keyboard_Class::KeysState& status, const HotkeyBinding& binding) {
    switch (binding.triggerType) {
        case HotkeyTriggerType::PrintableChar:
            return M5Cardputer.Keyboard.isKeyPressed(binding.printableKey);

        case HotkeyTriggerType::HidKey:
            return statusContainsHidKey(status, binding.triggerHidKey);

        default:
            return false;
    }
}

void copyBindingToReport(const HotkeyBinding& binding, HotkeyReport& report) {
    clearReport(report);
    report.modifiers = binding.modifiers;

    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = binding.outputKeys[i];
    }
}
}  // namespace

bool buildHotkeyReport(const Keyboard_Class::KeysState& status, HotkeyReport& report) {
    clearReport(report);

    for (int i = 0; i < kHotkeyBindingCount; ++i) {
        if (bindingTriggered(status, kHotkeyBindings[i])) {
            copyBindingToReport(kHotkeyBindings[i], report);
            return true;
        }
    }

    return false;
}
// SEGMENT C END — Hotkey Lookup And Report Builder