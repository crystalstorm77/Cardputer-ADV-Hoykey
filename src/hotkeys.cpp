// SEGMENT A START — Hotkey Includes And Types
#include "hotkeys.h"

constexpr uint8_t HID_KEY_A = 0x04;
constexpr uint8_t HID_KEY_Z = 0x1D;
constexpr uint8_t HID_KEY_1 = 0x1E;
constexpr uint8_t HID_KEY_2 = 0x1F;
constexpr uint8_t HID_KEY_3 = 0x20;
constexpr uint8_t HID_KEY_4 = 0x21;
constexpr uint8_t HID_KEY_5 = 0x22;
constexpr uint8_t HID_KEY_6 = 0x23;
constexpr uint8_t HID_KEY_7 = 0x24;
constexpr uint8_t HID_KEY_8 = 0x25;
constexpr uint8_t HID_KEY_9 = 0x26;
constexpr uint8_t HID_KEY_0 = 0x27;
constexpr uint8_t HID_KEY_ENTER = 0x28;
constexpr uint8_t HID_KEY_ESCAPE = 0x29;
constexpr uint8_t HID_KEY_BACKSPACE = 0x2A;
constexpr uint8_t HID_KEY_SPACE = 0x2C;
constexpr uint8_t HID_KEY_MINUS = 0x2D;
constexpr uint8_t HID_KEY_EQUAL = 0x2E;
constexpr uint8_t HID_KEY_LEFT_BRACKET = 0x2F;
constexpr uint8_t HID_KEY_RIGHT_BRACKET = 0x30;
constexpr uint8_t HID_KEY_BACKSLASH = 0x31;
constexpr uint8_t HID_KEY_SEMICOLON = 0x33;
constexpr uint8_t HID_KEY_APOSTROPHE = 0x34;
constexpr uint8_t HID_KEY_GRAVE = 0x35;
constexpr uint8_t HID_KEY_COMMA = 0x36;
constexpr uint8_t HID_KEY_PERIOD = 0x37;
constexpr uint8_t HID_KEY_SLASH = 0x38;
constexpr uint8_t HID_KEY_DELETE = 0x4C;
constexpr uint8_t HID_KEY_RIGHT_ARROW = 0x4F;
constexpr uint8_t HID_KEY_LEFT_ARROW = 0x50;
constexpr uint8_t HID_KEY_DOWN_ARROW = 0x51;
constexpr uint8_t HID_KEY_UP_ARROW = 0x52;
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
bool gKeyboardSymbolLayerEnabled = false;
bool gKeyboardShiftKeyWasPressed = false;

void clearHotkeyReport(HotkeyReport& report) {
    report.modifiers = 0;
    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = 0;
    }
}

void clearKeyboardModeReport(KeyboardModeReport& report) {
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

bool reportAlreadyContainsKey(const KeyboardModeReport& report, uint8_t hidKey) {
    for (uint8_t i = 0; i < 6; ++i) {
        if (report.keys[i] == hidKey) {
            return true;
        }
    }
    return false;
}

void addKeyToKeyboardModeReport(KeyboardModeReport& report, uint8_t hidKey) {
    if (hidKey == 0 || reportAlreadyContainsKey(report, hidKey)) {
        return;
    }

    for (uint8_t i = 0; i < 6; ++i) {
        if (report.keys[i] == 0) {
            report.keys[i] = hidKey;
            return;
        }
    }
}

bool isFnMappedHidKey(uint8_t hidKey) {
    switch (hidKey) {
        case HID_KEY_GRAVE:
        case HID_KEY_SEMICOLON:
        case HID_KEY_COMMA:
        case HID_KEY_PERIOD:
        case HID_KEY_SLASH:
            return true;
        default:
            return false;
    }
}

uint8_t mapFnLayerHidKey(uint8_t hidKey) {
    switch (hidKey) {
        case HID_KEY_GRAVE:
            return HID_KEY_ESCAPE;
        case HID_KEY_SEMICOLON:
            return HID_KEY_UP_ARROW;
        case HID_KEY_PERIOD:
            return HID_KEY_DOWN_ARROW;
        case HID_KEY_COMMA:
            return HID_KEY_LEFT_ARROW;
        case HID_KEY_SLASH:
            return HID_KEY_RIGHT_ARROW;
        default:
            return 0;
    }
}

bool shouldApplyShiftForSymbolLayer(uint8_t hidKey) {
    if (hidKey >= HID_KEY_A && hidKey <= HID_KEY_Z) {
        return true;
    }

    switch (hidKey) {
        case HID_KEY_1:
        case HID_KEY_2:
        case HID_KEY_3:
        case HID_KEY_4:
        case HID_KEY_5:
        case HID_KEY_6:
        case HID_KEY_7:
        case HID_KEY_8:
        case HID_KEY_9:
        case HID_KEY_0:
        case HID_KEY_MINUS:
        case HID_KEY_EQUAL:
        case HID_KEY_LEFT_BRACKET:
        case HID_KEY_RIGHT_BRACKET:
        case HID_KEY_BACKSLASH:
        case HID_KEY_SEMICOLON:
        case HID_KEY_APOSTROPHE:
        case HID_KEY_GRAVE:
        case HID_KEY_COMMA:
        case HID_KEY_PERIOD:
        case HID_KEY_SLASH:
            return true;
        default:
            return false;
    }
}

void updateKeyboardSymbolLayerToggle() {
    const bool shiftPressed = M5Cardputer.Keyboard.isKeyPressed(KEY_LEFT_SHIFT);

    if (shiftPressed && !gKeyboardShiftKeyWasPressed) {
        gKeyboardSymbolLayerEnabled = !gKeyboardSymbolLayerEnabled;
    }

    gKeyboardShiftKeyWasPressed = shiftPressed;
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
    clearHotkeyReport(report);
    report.modifiers = binding.modifiers;

    for (uint8_t i = 0; i < 6; ++i) {
        report.keys[i] = binding.outputKeys[i];
    }
}
}  // namespace

bool buildHotkeyReport(const Keyboard_Class::KeysState& status, HotkeyReport& report) {
    clearHotkeyReport(report);

    for (int i = 0; i < kHotkeyBindingCount; ++i) {
        if (bindingTriggered(status, kHotkeyBindings[i])) {
            copyBindingToReport(kHotkeyBindings[i], report);
            return true;
        }
    }

    return false;
}

void buildKeyboardModeReport(const Keyboard_Class::KeysState& status, KeyboardModeReport& report) {
    clearKeyboardModeReport(report);
    updateKeyboardSymbolLayerToggle();

    const bool fnPressed = status.fn;

    if (status.ctrl) {
        report.modifiers |= MODIFIER_LEFT_CTRL;
    }
    if (status.alt) {
        report.modifiers |= MODIFIER_LEFT_ALT;
    }

    for (auto key : status.hid_keys) {
        if (fnPressed && isFnMappedHidKey(key)) {
            addKeyToKeyboardModeReport(report, mapFnLayerHidKey(key));
            continue;
        }

        if (gKeyboardSymbolLayerEnabled && shouldApplyShiftForSymbolLayer(key)) {
            report.modifiers |= MODIFIER_LEFT_SHIFT;
        }

        addKeyToKeyboardModeReport(report, key);
    }

    if (status.enter) {
        addKeyToKeyboardModeReport(report, HID_KEY_ENTER);
    }

    if (status.del) {
        if (fnPressed) {
            addKeyToKeyboardModeReport(report, HID_KEY_DELETE);
        } else {
            addKeyToKeyboardModeReport(report, HID_KEY_BACKSPACE);
        }
    }

    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        addKeyToKeyboardModeReport(report, HID_KEY_SPACE);
    }
}

bool keyboardModeReportHasOutput(const KeyboardModeReport& report) {
    if (report.modifiers != 0) {
        return true;
    }

    for (uint8_t i = 0; i < 6; ++i) {
        if (report.keys[i] != 0) {
            return true;
        }
    }

    return false;
}

bool isKeyboardSymbolLayerEnabled() {
    return gKeyboardSymbolLayerEnabled;
}
// SEGMENT C END — Hotkey Lookup And Report Builder