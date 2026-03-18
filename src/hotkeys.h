// SEGMENT A START — Hotkey Types And Declarations
#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <stdint.h>
#include <M5Cardputer.h>

struct HotkeyReport {
    uint8_t modifiers = 0;
    uint8_t keys[6] = {0};
};

struct KeyboardModeReport {
    uint8_t modifiers = 0;
    uint8_t keys[6] = {0};
};

bool buildHotkeyReport(const Keyboard_Class::KeysState& status, HotkeyReport& report);

void buildKeyboardModeReport(const Keyboard_Class::KeysState& status, KeyboardModeReport& report);
bool keyboardModeReportHasOutput(const KeyboardModeReport& report);
bool isKeyboardSymbolLayerEnabled();

#endif
// SEGMENT A END — Hotkey Types And Declarations