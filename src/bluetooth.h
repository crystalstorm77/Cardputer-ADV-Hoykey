// SEGMENT A START — Bluetooth Includes And HID Report Map
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "display.h"

extern BLEHIDDevice* hid;
extern BLECharacteristic* mouseInput;
extern BLECharacteristic* keyboardInput;
extern bool bluetoothIsConnected;

const uint8_t HID_REPORT_MAP[] = {
    // Keyboard report only
    0x05, 0x01,
    0x09, 0x06,
    0xA1, 0x01,
    0x85, 0x01,
    0x05, 0x07,
    0x19, 0xE0,
    0x29, 0xE7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,
    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x26, 0x73, 0x00,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x73,
    0x81, 0x00,
    0xC0
};
// SEGMENT A END — Bluetooth Includes And HID Report Map

// SEGMENT B START — Bluetooth Function Declarations
void initBluetooth();
void deinitBluetooth();
bool getBluetoothStatus();

void bluetoothMouse();
void bluetoothKeyboard();
void bluetoothHotkey();
void sendEmptyReports();
void handleBluetoothMode(DeviceMode currentMode);

class MyBLEServerCallbacks : public BLEServerCallbacks {
public:
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) override;
};

#endif // BLUETOOTH_H
// SEGMENT B END — Bluetooth Function Declarations