// =====================================================================
//  BLE_UART.h  -  Lab 8 helper file (provided by your instructor)
//
//  Place this file in the "include" folder of your PlatformIO project.
//  You do NOT need to read or change anything in this file.
//
//  It turns the ESP32 into a Bluetooth Low Energy (BLE) device that
//  uses the Nordic UART Service, so phone apps such as
//  Bluefruit Connect (iPhone and Android) can send it text.
//
//  Functions you can use in main.cpp:
//
//    bleBegin("ESP32_Name");   Start BLE and let phones find the ESP32
//    bleIsConnected()          true if a phone is connected
//    bleMessageAvailable()     true if a new message has arrived
//    bleReadMessage()          returns the new message as a String
//    bleSend("text")           sends text back to the phone
// =====================================================================

#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Nordic UART Service UUIDs
#define UART_SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX_UUID       "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Phone -> ESP32
#define UART_TX_UUID       "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // ESP32 -> Phone

static BLEServer*         bleServer       = NULL;
static BLECharacteristic* bleTxChar       = NULL;
static bool               bleWasConnected = false;
static volatile bool      bleNewMessage   = false;
static String             bleLastMessage  = "";

// Runs automatically when the phone writes a message
class BleRxEvents : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic* characteristic)
  {
    bleLastMessage = characteristic->getValue().c_str();
    bleLastMessage.trim();        // remove spaces and line endings
    bleNewMessage = true;
  }
};

// Start BLE with a device name and begin advertising
void bleBegin(String deviceName)
{
  BLEDevice::init(deviceName.c_str());

  bleServer = BLEDevice::createServer();

  BLEService* service = bleServer->createService(UART_SERVICE_UUID);

  bleTxChar = service->createCharacteristic(
      UART_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  bleTxChar->addDescriptor(new BLE2902());

  BLECharacteristic* rxChar = service->createCharacteristic(
      UART_RX_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxChar->setCallbacks(new BleRxEvents());

  service->start();

  bleServer->getAdvertising()->addServiceUUID(UART_SERVICE_UUID);
  bleServer->getAdvertising()->start();
}

// true if a phone is connected
// The connection is read directly from the BLE server, so a disconnect
// is never missed. When the phone disconnects, advertising is restarted
// here (after a short pause) so the phone can find and reconnect to the
// ESP32. Restarting advertising inside the disconnect event itself can
// fail on some phones, leaving the ESP32 stuck.
bool bleIsConnected()
{
  if (bleServer == NULL)
  {
    return false;
  }

  bool connectedNow = bleServer->getConnectedCount() > 0;

  if (connectedNow == false && bleWasConnected == true)
  {
    delay(500);                     // give the BLE stack time to finish
    bleServer->startAdvertising();  // let a phone find the ESP32 again
  }

  bleWasConnected = connectedNow;
  return connectedNow;
}

// true if a new message has arrived from the phone
bool bleMessageAvailable()
{
  return bleNewMessage;
}

// Returns the newest message (and marks it as read)
String bleReadMessage()
{
  bleNewMessage = false;
  return bleLastMessage;
}

// Sends text to the phone (keep it under 20 characters)
void bleSend(String text)
{
  if (bleServer != NULL && bleServer->getConnectedCount() > 0 && bleTxChar != NULL)
  {
    bleTxChar->setValue(text.c_str());
    bleTxChar->notify();
  }
}
