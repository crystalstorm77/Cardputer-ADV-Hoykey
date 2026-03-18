// SEGMENT A START — Display Types And Includes
#ifndef DISPLAY_H
#define DISPLAY_H

#include <M5Cardputer.h>

enum class DeviceMode : uint8_t {
    Keyboard = 0,
    Mouse = 1,
    Hotkey = 2
};
// SEGMENT A END — Display Types And Includes

// SEGMENT B START — Display Function Declarations
void setupDisplay();
void displayWelcomeScreen();
void displaySelectionScreen(bool usbMode);
void displayMainScreen(bool usbMode, DeviceMode currentMode, bool bluetoothStatus, int batteryLevel);
void modeIndicator(bool usbMode, bool bluetoothStatus);
void drawModeCards(DeviceMode currentMode);
void drawBatteryLevel(int batteryLevel);
// SEGMENT B END — Display Function Declarations

#endif