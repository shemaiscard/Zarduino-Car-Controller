#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <HardwareSerial.h>

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
      Serial.println("BLE Device Disconnected");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue().c_str();// Use std::string for getValue()

      if (rxValue.length() > 0) {
        Serial.print("Received Command: ");
        Serial.println(rxValue.c_str()); // Print as C-string
        
        // Forward to Arduino
        // The Arduino code expects a single character for manual commands (f,s,l,r,b)
        // For joystick data (e.g., "J100,50"), you'll need to parse this on the Arduino.
        // For now, sending the raw string.
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
