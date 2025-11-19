#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define LED1 4
#define LED2 16
#define LED3 17

#define SERVICE_UUID           "12345678-1234-1234-1234-1234567890ab"
#define CHAR_READ_UUID         "12345678-1234-1234-1234-1234567890ac"
#define CHAR_WRITE_UUID        "12345678-1234-1234-1234-1234567890ad"

BLECharacteristic *readChar;
BLECharacteristic *writeChar;

class LEDWriteCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = std::string(pCharacteristic->getValue().c_str());

    if (value.length() > 0) {
      char cmd = value[0];

      // Điều khiển LED tương ứng
      if (cmd == '1') digitalWrite(LED1, !digitalRead(LED1));
      if (cmd == '2') digitalWrite(LED2, !digitalRead(LED2));
      if (cmd == '3') digitalWrite(LED3, !digitalRead(LED3));

      Serial.print("Received BLE:");
      Serial.println(cmd);
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Setup GPIO
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  // BLE Setup
  BLEDevice::init("ESP32_BLE");
  BLEServer *server = BLEDevice::createServer();

  BLEService *service = server->createService(SERVICE_UUID);

  // Characteristic READ (xem trạng thái LED)
  readChar = service->createCharacteristic(
               CHAR_READ_UUID,
               BLECharacteristic::PROPERTY_READ
             );

  // Characteristic WRITE (điều khiển LED)
  writeChar = service->createCharacteristic(
                CHAR_WRITE_UUID,
                BLECharacteristic::PROPERTY_WRITE
              );

  writeChar->setCallbacks(new LEDWriteCallback());

  service->start();

  // Bật quảng bá BLE
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("BLE started! Waiting for connections...");
}

void loop() {
  // Luôn cập nhật trạng thái LED để app đọc được
  String state = 
      String(digitalRead(LED1)) + "," +
      String(digitalRead(LED2)) + "," +
      String(digitalRead(LED3));

  readChar->setValue(state.c_str());
  delay(200);
}
