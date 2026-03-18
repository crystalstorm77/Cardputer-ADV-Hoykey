// SEGMENT A START — USB HID Includes And Declarations
#ifndef USBHID_H
#define USBHID_H

#include "display.h"

void usbMouse();
void usbKeyboard();
void usbHotkey();
void handleUsbMode(DeviceMode currentMode);

#endif
// SEGMENT A END — USB HID Includes And Declarations