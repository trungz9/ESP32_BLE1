#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>

// UUID của thiết bị Peripheral (ESP32 kia)
#define SERVICE_UUID           "12345678-1234-1234-1234-1234567890ab"
#define CHAR_READ_UUID         "12345678-1234-1234-1234-1234567890ac"
#define CHAR_WRITE_UUID        "12345678-1234-1234-1234-1234567890ad"

BLEScan* pBLEScan;
BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharRead;
BLERemoteCharacteristic* pRemoteCharWrite;

bool deviceFound = false;
BLEAddress* pServerAddress;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found device: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Kiểm tra xem có phải thiết bị cần tìm không (theo tên hoặc UUID)
    if (advertisedDevice.getName() == "ESP32_BLE") {
      Serial.println("-> Target device found!");
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      deviceFound = true;
      advertisedDevice.getScan()->stop();
    }
  }
};

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify received: ");
  Serial.write(pData, length);
  Serial.println();
}


bool connectToServer() {
  Serial.print("Connecting to ");
  Serial.println(pServerAddress->toString().c_str());

  pClient = BLEDevice::createClient();
  
  if (!pClient->connect(*pServerAddress)) {
    Serial.println("Failed to connect");
    return false;
  }
  Serial.println("Connected!");

  // Lấy service
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.println("Service not found");
    pClient->disconnect();
    return false;
  }

  // Lấy characteristic READ
  pRemoteCharRead = pRemoteService->getCharacteristic(CHAR_READ_UUID);
  if (pRemoteCharRead == nullptr) {
    Serial.println("Read characteristic not found");
    pClient->disconnect();
    return false;
  }

  // Lấy characteristic WRITE
  pRemoteCharWrite = pRemoteService->getCharacteristic(CHAR_WRITE_UUID);
  if (pRemoteCharWrite == nullptr) {
    Serial.println("Write characteristic not found");
    pClient->disconnect();
    return false;
  }

  // Đăng ký nhận notify (nếu Peripheral hỗ trợ)
  if (pRemoteCharRead->canNotify()) {
    pRemoteCharRead->registerForNotify(notifyCallback);
    Serial.println("Registered for notifications");
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("BLE Central starting...");

  BLEDevice::init("ESP32_Central");

  // Quét thiết bị BLE
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5); // Quét trong 5 giây
}

void loop() {
  // Nếu tìm thấy thiết bị và chưa kết nối
  if (deviceFound && !pClient->isConnected()) {
    if (connectToServer()) {
      Serial.println("Ready to read/write!");
    } else {
      Serial.println("Connection failed, rescanning...");
      deviceFound = false;
      pBLEScan->start(5);
    }
  }

  // Nếu đã kết nối, thực hiện đọc/ghi
  if (pClient != nullptr && pClient->isConnected()) {
    
    // ĐỌC giá trị (trạng thái LED)
    if (pRemoteCharRead != nullptr) {
      std::string value = pRemoteCharRead->readValue();
      Serial.print("LED Status: ");
      Serial.println(value.c_str());
    }

    // GHI lệnh điều khiển (ví dụ: toggle LED1)
    if (pRemoteCharWrite != nullptr) {
      pRemoteCharWrite->writeValue("1", 1); // Gửi ký tự '1'
      Serial.println("Sent command: 1");
    }

    delay(3000); // Đọc/ghi mỗi 3 giây
  } else {
    delay(1000);
  }
}