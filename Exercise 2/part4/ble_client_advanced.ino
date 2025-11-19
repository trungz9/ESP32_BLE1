#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHAR_NOTIFY_UUID    "12345678-1234-1234-1234-1234567890ac"
#define CHAR_LONG_UUID      "12345678-1234-1234-1234-1234567890ad"

BLEScan* pBLEScan;
BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharNotify;
BLERemoteCharacteristic* pRemoteCharLong;

bool deviceFound = false;
bool connected = false;
BLEAddress* pServerAddress;

String longStringBuffer = ""; // Buffer để nối chuỗi dài

// ====================================
// Security Callback
// ====================================
class MyClientSecurity : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    Serial.println("🔐 Enter passkey: 123456");
    return 123456;
  }

  void onPassKeyNotify(uint32_t pass_key) {
    Serial.print("🔑 Passkey: ");
    Serial.println(pass_key);
  }

  bool onConfirmPIN(uint32_t pass_key) {
    Serial.print("🔐 Confirm PIN: ");
    Serial.println(pass_key);
    return true;
  }

  bool onSecurityRequest() {
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
    if (cmpl.success) {
      Serial.println("✅ Authentication successful!");
    } else {
      Serial.println("❌ Authentication failed!");
    }
  }
};

// ====================================
// Callback nhận NOTIFY (dữ liệu thay đổi)
// ====================================
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  
  Serial.print("📊 Notify: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)pData[i]);
  }
  Serial.println();
}

// ====================================
// Callback nhận CHUỖI DÀI
// ====================================
static void longStringCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  
  String chunk = "";
  for (int i = 0; i < length; i++) {
    chunk += (char)pData[i];
  }

  // Kiểm tra tín hiệu kết thúc
  if (chunk == "END") {
    Serial.println("\n📥 Complete message:");
    Serial.println(longStringBuffer);
    Serial.print("Total length: ");
    Serial.print(longStringBuffer.length());
    Serial.println(" bytes\n");
    longStringBuffer = ""; // Reset buffer
  } else {
    longStringBuffer += chunk;
    Serial.print("📦 Chunk received: ");
    Serial.println(chunk);
  }
}

// ====================================
// Tìm thiết bị BLE
// ====================================
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.getName() == "ESP32_Secure") {
      Serial.println("🎯 Target found!");
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      deviceFound = true;
      advertisedDevice.getScan()->stop();
    }
  }
};

// ====================================
// Kết nối tới Server
// ====================================
bool connectToServer() {
  Serial.print("🔗 Connecting to ");
  Serial.println(pServerAddress->toString().c_str());

  pClient = BLEDevice::createClient();
  
  // Cấu hình security
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setCapability(ESP_IO_CAP_KBDISP);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

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

  // 1. Lấy NOTIFY characteristic
  pRemoteCharNotify = pRemoteService->getCharacteristic(CHAR_NOTIFY_UUID);
  if (pRemoteCharNotify == nullptr) {
    Serial.println("❌ Notify characteristic not found");
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharNotify->canNotify()) {
    pRemoteCharNotify->registerForNotify(notifyCallback);
    Serial.println("✅ Registered for notify");
  }

  // 2. Lấy LONG STRING characteristic
  pRemoteCharLong = pRemoteService->getCharacteristic(CHAR_LONG_UUID);
  if (pRemoteCharLong == nullptr) {
    Serial.println("❌ Long string characteristic not found");
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharLong->canNotify()) {
    pRemoteCharLong->registerForNotify(longStringCallback);
    Serial.println("✅ Registered for long string");
  }

  return true;
}

// ====================================
// SETUP
// ====================================
void setup() {
  Serial.begin(115200);
  Serial.println("🚀 BLE Client Advanced starting...");

  BLEDevice::init("ESP32_Client");
  
  // Cấu hình security
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new MyClientSecurity());

  // Quét thiết bị
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);
}

// ====================================
// LOOP
// ====================================
void loop() {
  if (deviceFound && !connected) {
    if (connectToServer()) {
      connected = true;
      Serial.println("🎉 Ready to receive data!");
    } else {
      Serial.println("⏳ Rescanning...");
      deviceFound = false;
      pBLEScan->start(10);
    }
  }

  if (connected && (pClient == nullptr || !pClient->isConnected())) {
    Serial.println("❌ Disconnected! Rescanning...");
    connected = false;
    deviceFound = false;
    pBLEScan->start(10);
  }

  delay(1000);
}