// SEGMENT A START — Bluetooth Includes And Global State
#include "bluetooth.h"
#include "hotkeys.h"

BLEHIDDevice* hid;
BLECharacteristic* mouseInput;
BLECharacteristic* keyboardInput;
bool bluetoothIsConnected = false;

namespace {
void sendKeyboardReport(uint8_t modifier, const uint8_t keycode[6]) {
    uint8_t report[8] = {
        modifier, 0,
        keycode[0], keycode[1], keycode[2],
        keycode[3], keycode[4], keycode[5]
    };
    keyboardInput->setValue(report, sizeof(report));
    keyboardInput->notify();
}
}  // namespace
// SEGMENT A END — Bluetooth Includes And Global State

// SEGMENT B START — Bluetooth Input Modes
void MyBLEServerCallbacks::onConnect(BLEServer* pServer) {
    bluetoothIsConnected = true;
}

void MyBLEServerCallbacks::onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) {
    bluetoothIsConnected = false;

    pServer->disconnect(param->disconnect.conn_id);
    pServer->startAdvertising();
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
    mouseInput->setValue(report, sizeof(report));
    mouseInput->notify();
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
    uint8_t emptyMouseReport[4] = {0, 0, 0, 0};
    mouseInput->setValue(emptyMouseReport, sizeof(emptyMouseReport));
    mouseInput->notify();

    uint8_t emptyKeyboardReport[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    keyboardInput->setValue(emptyKeyboardReport, sizeof(emptyKeyboardReport));
    keyboardInput->notify();
}

void handleBluetoothMode(DeviceMode currentMode) {
    if (!bluetoothIsConnected) {
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
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyBLEServerCallbacks());

    hid = new BLEHIDDevice(pServer);
    mouseInput = hid->inputReport(1);
    keyboardInput = hid->inputReport(2);

    hid->manufacturer()->setValue("M5Stack");
    hid->pnp(0x02, 0x1234, 0x5678, 0x0100);
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t*)HID_REPORT_MAP, sizeof(HID_REPORT_MAP));
    hid->startServices();

    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_MOUSE);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();

    BLESecurity* pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

void deinitBluetooth() {
    BLEDevice::deinit();
    delay(1000);
}
// SEGMENT C END — Bluetooth Setup And Teardown