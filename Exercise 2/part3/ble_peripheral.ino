#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHAR_TEMP_UUID      "12345678-1234-1234-1234-1234567890ac"

BLECharacteristic *tempCharacteristic;
float temperature = 25.0; // Nhiệt độ giả lập

void setup() {
  Serial.begin(115200);
  Serial.println("BLE Peripheral starting...");

  // Khởi tạo BLE
  BLEDevice::init("ESP32_Sensor");
  
  // Tạo BLE Server
  BLEServer *server = BLEDevice::createServer();
  
  // Tạo Service
  BLEService *service = server->createService(SERVICE_UUID);
  
  // Tạo Characteristic (READ + NOTIFY)
  tempCharacteristic = service->createCharacteristic(
    CHAR_TEMP_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  
  // Đặt giá trị ban đầu
  tempCharacteristic->setValue(String(temperature).c_str());
  
  // Khởi động service
  service->start();
  
  // Bắt đầu quảng bá
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();
  
  Serial.println("BLE Peripheral ready! Broadcasting...");
}

void loop() {
  // Giả lập nhiệt độ thay đổi
  temperature = 20.0 + (rand() % 100) / 10.0; // 20.0 - 30.0°C
  
  // Cập nhật giá trị
  String tempStr = String(temperature, 1) + "C";
  tempCharacteristic->setValue(tempStr.c_str());
  tempCharacteristic->notify(); // Gửi notify cho client
  
  Serial.print("Temperature: ");
  Serial.println(tempStr);
  
  delay(2000); // Cập nhật mỗi 2 giây
}