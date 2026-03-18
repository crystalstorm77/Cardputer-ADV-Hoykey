// SEGMENT A START — Bluetooth Includes And Global State
#include "bluetooth.h"
#include <BLESecurity.h>
#include <BLE2902.h>

#include "hotkeys.h"

BLEHIDDevice* hid = nullptr;
BLECharacteristic* mouseInput = nullptr;
BLECharacteristic* keyboardInput = nullptr;
bool bluetoothIsConnected = false;

namespace {
BLEServer* bleServer = nullptr;
BLEAdvertising* bleAdvertising = nullptr;
BLE2902* mouseInput2902 = nullptr;
BLE2902* keyboardInput2902 = nullptr;

bool bluetoothAdvertising = false;
bool bluetoothNeedsNotificationRearm = false;

unsigned long lastBleAdvertisingRefreshMs = 0;
unsigned long lastBleNotificationRearmMs = 0;

constexpr unsigned long kBleAdvertisingRefreshMs = 2000UL;
constexpr unsigned long kBleNotificationRearmDelayMs = 250UL;

void sendKeyboardReport(uint8_t modifier, const uint8_t keycode[6]) {
    if (keyboardInput == nullptr) {
        return;
    }

    uint8_t report[8] = {
        modifier, 0,
        keycode[0], keycode[1], keycode[2],
        keycode[3], keycode[4], keycode[5]
    };

    keyboardInput->setValue(report, sizeof(report));
    keyboardInput->notify();
}

void startBleAdvertisingInternal() {
    if (bleAdvertising == nullptr) {
        return;
    }

    bleAdvertising->stop();
    delay(30);
    bleAdvertising->start();

    bluetoothAdvertising = true;
    lastBleAdvertisingRefreshMs = millis();
}

void stopBleAdvertisingInternal() {
    if (bleAdvertising == nullptr) {
        return;
    }

    bleAdvertising->stop();
    bluetoothAdvertising = false;
}

void ensureBleAdvertising() {
    if (bluetoothIsConnected || bleAdvertising == nullptr) {
        return;
    }

    const unsigned long now = millis();
    if (!bluetoothAdvertising || (now - lastBleAdvertisingRefreshMs) >= kBleAdvertisingRefreshMs) {
        startBleAdvertisingInternal();
    }
}

void setDescriptorNotifications(BLE2902* descriptor, bool enabled) {
    if (descriptor == nullptr) {
        return;
    }

    descriptor->setNotifications(enabled);
    descriptor->setIndications(false);
}

void rearmInputNotificationsIfNeeded() {
    if (!bluetoothIsConnected || !bluetoothNeedsNotificationRearm) {
        return;
    }

    const unsigned long now = millis();
    if ((now - lastBleNotificationRearmMs) < kBleNotificationRearmDelayMs) {
        return;
    }

    setDescriptorNotifications(mouseInput2902, true);
    setDescriptorNotifications(keyboardInput2902, true);

    bluetoothNeedsNotificationRearm = false;
    sendEmptyReports();
}
}  // namespace
// SEGMENT A END — Bluetooth Includes And Global State

// SEGMENT B START — Bluetooth Input Modes
void MyBLEServerCallbacks::onConnect(BLEServer* pServer) {
    bluetoothIsConnected = true;
    bluetoothAdvertising = false;
    bluetoothNeedsNotificationRearm = true;
    lastBleNotificationRearmMs = millis();
}

void MyBLEServerCallbacks::onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) {
    bluetoothIsConnected = false;
    bluetoothAdvertising = false;
    bluetoothNeedsNotificationRearm = false;
    sendEmptyReports();

    delay(50);
    startBleAdvertisingInternal();
}

bool getBluetoothStatus() {
    return bluetoothIsConnected;
}

void bluetoothMouse() {
    int16_t x = 0;
    int16_t y = 0;
    uint8_t buttons = 0;

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (status.enter) {
        buttons |= 0x01;
    }

    if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
        buttons |= 0x02;
    }

    if (M5Cardputer.Keyboard.isKeyPressed(';')) {
        y -= 1;
    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
        y += 1;
    }

    if (M5Cardputer.Keyboard.isKeyPressed('/')) {
        x += 1;
    } else if (M5Cardputer.Keyboard.isKeyPressed(',')) {
        x -= 1;
    }

    uint8_t report[4] = {buttons, static_cast<uint8_t>(x), static_cast<uint8_t>(y), 0};

    if (mouseInput != nullptr) {
        mouseInput->setValue(report, sizeof(report));
        mouseInput->notify();
    }
}

void bluetoothKeyboard() {
    uint8_t modifier = 0;
    uint8_t keycode[6] = {0};

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    int count = 0;
    for (auto key : status.hid_keys) {
        if (count < 6) {
            keycode[count++] = key;
        } else {
            break;
        }
    }

    if (M5Cardputer.Keyboard.isKeyPressed(' ') && count < 6) {
        keycode[count++] = 0x2C;
    }

    if (status.ctrl)  modifier |= 0x01;
    if (status.shift) modifier |= 0x02;
    if (status.alt)   modifier |= 0x04;

    sendKeyboardReport(modifier, keycode);
}

void bluetoothHotkey() {
    HotkeyReport hotkeyReport = {};
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (buildHotkeyReport(status, hotkeyReport)) {
        sendKeyboardReport(hotkeyReport.modifiers, hotkeyReport.keys);
    } else {
        sendEmptyReports();
    }
}

void sendEmptyReports() {
    if (mouseInput != nullptr) {
        uint8_t emptyMouseReport[4] = {0, 0, 0, 0};
        mouseInput->setValue(emptyMouseReport, sizeof(emptyMouseReport));
        mouseInput->notify();
    }

    if (keyboardInput != nullptr) {
        uint8_t emptyKeyboardReport[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        keyboardInput->setValue(emptyKeyboardReport, sizeof(emptyKeyboardReport));
        keyboardInput->notify();
    }
}

void handleBluetoothMode(DeviceMode currentMode) {
    ensureBleAdvertising();
    rearmInputNotificationsIfNeeded();

    if (!bluetoothIsConnected) {
        delay(7);
        return;
    }

    if (bluetoothNeedsNotificationRearm) {
        delay(7);
        return;
    }

    if (!M5Cardputer.Keyboard.isPressed()) {
        sendEmptyReports();
        delay(7);
        return;
    }

    switch (currentMode) {
        case DeviceMode::Mouse:
            bluetoothMouse();
            break;
        case DeviceMode::Hotkey:
            bluetoothHotkey();
            break;
        case DeviceMode::Keyboard:
        default:
            bluetoothKeyboard();
            break;
    }

    delay(7);
}
// SEGMENT B END — Bluetooth Input Modes

// SEGMENT C START — Bluetooth Setup And Teardown
void initBluetooth() {
    BLEDevice::init("M5-Keyboard-Mouse");

    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new MyBLEServerCallbacks());

    hid = new BLEHIDDevice(bleServer);
    mouseInput = hid->inputReport(1);
    keyboardInput = hid->inputReport(2);

    BLEDescriptor* mouseDescriptor = mouseInput->getDescriptorByUUID("2902");
    if (mouseDescriptor == nullptr) {
        mouseInput2902 = new BLE2902();
        mouseInput->addDescriptor(mouseInput2902);
    } else {
        mouseInput2902 = static_cast<BLE2902*>(mouseDescriptor);
    }

    BLEDescriptor* keyboardDescriptor = keyboardInput->getDescriptorByUUID("2902");
    if (keyboardDescriptor == nullptr) {
        keyboardInput2902 = new BLE2902();
        keyboardInput->addDescriptor(keyboardInput2902);
    } else {
        keyboardInput2902 = static_cast<BLE2902*>(keyboardDescriptor);
    }

    setDescriptorNotifications(mouseInput2902, true);
    setDescriptorNotifications(keyboardInput2902, true);

    hid->manufacturer()->setValue("M5Stack");
    hid->pnp(0x02, 0x1234, 0x5678, 0x0100);
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t*)HID_REPORT_MAP, sizeof(HID_REPORT_MAP));
    hid->startServices();

    bleAdvertising = bleServer->getAdvertising();
    bleAdvertising->setAppearance(HID_KEYBOARD);
    bleAdvertising->setScanResponse(true);
    bleAdvertising->addServiceUUID(hid->hidService()->getUUID());

    BLESecurity* pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);
    pSecurity->setKeySize(16);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    bluetoothNeedsNotificationRearm = false;
    startBleAdvertisingInternal();
}

void deinitBluetooth() {
    stopBleAdvertisingInternal();

    bluetoothIsConnected = false;
    bluetoothNeedsNotificationRearm = false;

    mouseInput2902 = nullptr;
    keyboardInput2902 = nullptr;
    bleAdvertising = nullptr;
    bleServer = nullptr;

    BLEDevice::deinit();
    delay(1000);
}
// SEGMENT C END — Bluetooth Setup And Teardown