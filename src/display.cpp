// SEGMENT A START — Display Constants And Helpers
#include "display.h"

namespace {
constexpr int kTitleX = 10;
constexpr int kTitleY = 10;
constexpr int kTitleW = 220;
constexpr int kTitleH = 20;

constexpr int kStatusX = 10;
constexpr int kStatusY = 39;
constexpr int kStatusW = 104;
constexpr int kStatusH = 20;

constexpr int kSwitchX = 123;
constexpr int kSwitchY = 39;
constexpr int kSwitchW = 106;
constexpr int kSwitchH = 20;

constexpr int kCardY = 70;
constexpr int kCardH = 80;
constexpr int kCardW = 66;
constexpr int kCardGap = 10;
constexpr int kCard1X = 10;
constexpr int kCard2X = kCard1X + kCardW + kCardGap;
constexpr int kCard3X = kCard2X + kCardW + kCardGap;

void drawCardLabel(int x, const char* label, bool selected) {
    const uint16_t borderColor = selected ? TFT_GREEN : TFT_WHITE;
    const uint16_t textColor = selected ? TFT_GREEN : TFT_LIGHTGREY;

    M5Cardputer.Display.drawRoundRect(x, kCardY, kCardW, kCardH, 4, borderColor);
    M5Cardputer.Display.setTextColor(textColor);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(x + 10, kCardY + 54);
    M5Cardputer.Display.print(label);
}

void drawKeyboardGlyph(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawRect(x, y, 28, 18, color);
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            M5Cardputer.Display.fillRect(x + 3 + (col * 5), y + 3 + (row * 6), 3, 3, color);
        }
    }
}

void drawMouseGlyph(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawRoundRect(x + 6, y, 16, 24, 5, color);
    M5Cardputer.Display.drawLine(x + 14, y + 2, x + 14, y + 11, color);
    M5Cardputer.Display.drawLine(x + 8, y + 12, x + 20, y + 12, color);
}

void drawHotkeyGlyph(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawRect(x, y, 28, 18, color);
    M5Cardputer.Display.setTextColor(color);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(x + 4, y + 5);
    M5Cardputer.Display.print("HK");
}

void clearBatteryArea() {
    M5Cardputer.Display.fillRect(176, 12, 44, 16, TFT_LIGHTGREY);
}
}  // namespace
// SEGMENT A END — Display Constants And Helpers

// SEGMENT B START — Mode Status Drawing
void drawBatteryLevel(int batteryLevel) {
    if (batteryLevel < 0) {
        batteryLevel = 0;
    }
    if (batteryLevel > 100) {
        batteryLevel = 100;
    }

    clearBatteryArea();
    M5Cardputer.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(184, 16);
    M5Cardputer.Display.printf("%3d%%", batteryLevel);
}

void drawModeCards(DeviceMode currentMode) {
    M5Cardputer.Display.fillRect(0, kCardY - 2, M5Cardputer.Display.width(), kCardH + 8, TFT_BLACK);

    drawCardLabel(kCard1X, "Keyboard", currentMode == DeviceMode::Keyboard);
    drawCardLabel(kCard2X, "Mouse", currentMode == DeviceMode::Mouse);
    drawCardLabel(kCard3X, "Hotkey", currentMode == DeviceMode::Hotkey);

    drawKeyboardGlyph(kCard1X + 19, kCardY + 16, currentMode == DeviceMode::Keyboard ? TFT_GREEN : TFT_WHITE);
    drawMouseGlyph(kCard2X + 19, kCardY + 12, currentMode == DeviceMode::Mouse ? TFT_GREEN : TFT_WHITE);
    drawHotkeyGlyph(kCard3X + 19, kCardY + 16, currentMode == DeviceMode::Hotkey ? TFT_GREEN : TFT_WHITE);
}

void modeIndicator(bool usbMode, bool bluetoothStatus) {
    M5Cardputer.Display.fillRect(kStatusX, kStatusY, kStatusW, kStatusH, TFT_BLACK);
    M5Cardputer.Display.setTextSize(1);

    if (bluetoothStatus || usbMode) {
        M5Cardputer.Display.drawRoundRect(kStatusX, kStatusY, kStatusW, kStatusH, 5, TFT_GREEN);
        M5Cardputer.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
        M5Cardputer.Display.drawRoundRect(kStatusX, kStatusY, kStatusW, kStatusH, 5, TFT_RED);
        M5Cardputer.Display.setTextColor(TFT_RED, TFT_BLACK);
    }

    if (usbMode) {
        M5Cardputer.Display.setCursor(49, 45);
        M5Cardputer.Display.print("USB");
    } else {
        M5Cardputer.Display.setCursor(24, 45);
        M5Cardputer.Display.print("Bluetooth");
    }
}
// SEGMENT B END — Mode Status Drawing

// SEGMENT C START — Screen Setup And Main Rendering
void setupDisplay() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_BLACK);
}

void displayWelcomeScreen() {
    M5Cardputer.Display.drawRect(9, 47, 220, 40, TFT_LIGHTGRAY);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(18, 58);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.printf("M5-Keyboard-Mouse");
    M5Cardputer.Display.setCursor(70, 120);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.printf("Version 1.1 - Geo");
    delay(2000);
}

void displayMainScreen(bool usbMode, DeviceMode currentMode, bool bluetoothStatus, int batteryLevel) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);

    M5Cardputer.Display.fillRoundRect(kTitleX, kTitleY, kTitleW, kTitleH, 5, TFT_LIGHTGREY);
    M5Cardputer.Display.setCursor(18, 14);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    M5Cardputer.Display.print("M5 Input Modes");

    drawBatteryLevel(batteryLevel);

    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(kSwitchX, kSwitchY, kSwitchW, kSwitchH, 5, TFT_LIGHTGREY);
    M5Cardputer.Display.setCursor(136, 45);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.print("GO switch");

    modeIndicator(usbMode, bluetoothStatus);
    drawModeCards(currentMode);
}

void displaySelectionScreen(bool usbMode) {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(70, 10);
    M5Cardputer.Display.printf("Select Mode:");

    M5Cardputer.Display.setTextSize(3);

    if (usbMode) {
        M5Cardputer.Display.fillRect(20, 30, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.drawRect(20, 30, 200, 40, TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_BLACK);
    } else {
        M5Cardputer.Display.fillRect(20, 30, 200, 40, TFT_BLACK);
        M5Cardputer.Display.drawRect(20, 30, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    }
    M5Cardputer.Display.setCursor(95, 40);
    M5Cardputer.Display.printf("USB");

    if (!usbMode) {
        M5Cardputer.Display.fillRect(20, 80, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.drawRect(20, 80, 200, 40, TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_BLACK);
    } else {
        M5Cardputer.Display.fillRect(20, 80, 200, 40, TFT_BLACK);
        M5Cardputer.Display.drawRect(20, 80, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    }
    M5Cardputer.Display.setCursor(42, 90);
    M5Cardputer.Display.printf("Bluetooth");
}
// SEGMENT C END — Screen Setup And Main Rendering