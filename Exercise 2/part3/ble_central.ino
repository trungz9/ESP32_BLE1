#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHAR_TEMP_UUID      "12345678-1234-1234-1234-1234567890ac"

BLEScan* pBLEScan;
BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharTemp;

bool deviceFound = false;
bool connected = false;
BLEAddress* pServerAddress;

// ====================================
// Callback khi tìm thấy thiết bị BLE
// ====================================
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.getName() == "ESP32_Sensor") {
      Serial.println("-> Target found!");
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      deviceFound = true;
      advertisedDevice.getScan()->stop();
    }
  }
};

// ====================================
// Callback khi nhận Notify
// ====================================
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  
  Serial.print("📊 Temperature: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)pData[i]);
  }
  Serial.println();
}

// ====================================
// Kết nối tới Server
// ====================================
bool connectToServer() {
  Serial.print("Connecting to ");
  Serial.println(pServerAddress->toString().c_str());

  pClient = BLEDevice::createClient();
  
  if (!pClient->connect(*pServerAddress)) {
    Serial.println("❌ Failed to connect");
    return false;
  }
  Serial.println("✅ Connected!");

  // Lấy service
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.println("❌ Service not found");
    pClient->disconnect();
    return false;
  }

  // Lấy characteristic
  pRemoteCharTemp = pRemoteService->getCharacteristic(CHAR_TEMP_UUID);
  if (pRemoteCharTemp == nullptr) {
    Serial.println("❌ Characteristic not found");
    pClient->disconnect();
    return false;
  }

  // Đăng ký nhận notify
  if (pRemoteCharTemp->canNotify()) {
    pRemoteCharTemp->registerForNotify(notifyCallback);
    Serial.println("✅ Registered for notifications");
  }

  return true;
}

// ====================================
// SETUP
// ====================================
void setup() {
  Serial.begin(115200);
  Serial.println("BLE Central starting...");

  BLEDevice::init("ESP32_Central");

  // Quét thiết bị
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10); // Quét 10 giây
}

// ====================================
// LOOP
// ====================================
void loop() {
  // Nếu tìm thấy nhưng chưa kết nối
  if (deviceFound && !connected) {
    if (connectToServer()) {
      connected = true;
      Serial.println("🔗 Ready to receive data!");
    } else {
      Serial.println("Rescanning...");
      deviceFound = false;
      pBLEScan->start(10);
    }
  }

  // Nếu mất kết nối
  if (connected && (pClient == nullptr || !pClient->isConnected())) {
    Serial.println("❌ Disconnected! Rescanning...");
    connected = false;
    deviceFound = false;
    pBLEScan->start(10);
  }

  delay(1000);
}
```

---

## 🎯 Cách sử dụng:

### Bước 1: Upload code
1. Upload **`ble_peripheral.ino`** vào **ESP32 #1**
2. Upload **`ble_central.ino`** vào **ESP32 #2**

### Bước 2: Mở Serial Monitor
- **ESP32 #1** (Peripheral): Thấy nhiệt độ giả lập mỗi 2 giây
- **ESP32 #2** (Central): Thấy:
```
  BLE Central starting...
  Found: ESP32_Sensor
  -> Target found!
  Connecting to xx:xx:xx:xx:xx:xx
  ✅ Connected!
  ✅ Registered for notifications
  🔗 Ready to receive data!
  📊 Temperature: 23.5C
  📊 Temperature: 26.2C
  📊 Temperature: 24.8C