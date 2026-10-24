#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLESecurity.h>
#include <HardwareSerial.h>

// Optional pairing: set REQUIRE_PASSKEY to 1 and choose your own 6-digit
// passkey. The browser then asks the operating system to pair, and only a
// device that enters the passkey can drive the car. Leave it at 0 for open
// classroom use (any nearby device can connect while the car is free).
#define REQUIRE_PASSKEY 0
#define PASSKEY 123456

// Use UART2 on different pins (GPIO16=RX, GPIO17=TX)
HardwareSerial SerialPort(2);

// BLE Setup
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Device Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      // Stop the car at once instead of letting it run on its last command.
      // (The Arduino's own watchdog stops it too if this byte is lost.)
      SerialPort.print('s');
      SerialPort.flush();
      Serial.println("BLE Device Disconnected - sent stop");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue().c_str();

      if (rxValue.length() > 0) {
        Serial.print("Received Command: ");
        Serial.println(rxValue.c_str());

        // Forward to Arduino: single characters (f, b, l, r, s) or an analog
        // joystick command such as "J60,-40\n", which the Arduino parses.
        SerialPort.print(rxValue.c_str());
        SerialPort.flush(); // Ensure data is sent
      }
    }
};

void setup() {
  Serial.begin(115200);
  SerialPort.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  BLEDevice::init("Arduino-BLE-Bridge");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

#if REQUIRE_PASSKEY
  // Writes need an encrypted, authenticated (passkey) link.
  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_WRITE_ENC_MITM);
  // The ESP32 has no screen or keypad, so it acts as "display only" with a
  // fixed passkey; the phone or computer asks the user to type it.
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setPassKey(true, PASSKEY);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setAuthenticationMode(true, true, true); // bonding, MITM protection, Secure Connections
#endif

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("BLE Bridge Ready - Waiting for connection...");
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarting BLE Advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(10);
}
