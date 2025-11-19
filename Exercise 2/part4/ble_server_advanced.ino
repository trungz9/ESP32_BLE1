#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLESecurity.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHAR_NOTIFY_UUID    "12345678-1234-1234-1234-1234567890ac"
#define CHAR_LONG_UUID      "12345678-1234-1234-1234-1234567890ad"

BLEServer *pServer;
BLECharacteristic *notifyChar;
BLECharacteristic *longChar;
bool deviceConnected = false;

// ====================================
// Security Callback (Passkey pairing)
// ====================================
class MySecurity : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    Serial.println("🔐 Passkey requested");
    return 123456; // Passkey cố định
  }

  void onPassKeyNotify(uint32_t pass_key) {
    Serial.print("🔑 Passkey: ");
    Serial.println(pass_key);
  }

  bool onConfirmPIN(uint32_t pass_key) {
    Serial.print("🔐 Confirm PIN: ");
    Serial.println(pass_key);
    return true; // Auto confirm
  }

  bool onSecurityRequest() {
    Serial.println("🔒 Security request");
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
    if (cmpl.success) {
      Serial.println("✅ Pairing successful!");
    } else {
      Serial.println("❌ Pairing failed!");
    }
  }
};

// ====================================
// Server Callback (kết nối/ngắt kết nối)
// ====================================
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("✅ Client connected!");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("❌ Client disconnected!");
    // Bắt đầu quảng bá lại
    BLEDevice::startAdvertising();
  }
};

// ====================================
// Gửi chuỗi dài (split packets)
// ====================================
void sendLongString(String data) {
  int dataLen = data.length();
  int chunkSize = 20; // MTU mặc định = 23, trừ 3 byte header = 20
  int chunks = (dataLen + chunkSize - 1) / chunkSize;

  Serial.println("📤 Sending long string...");
  Serial.print("Total length: ");
  Serial.print(dataLen);
  Serial.print(" bytes, ");
  Serial.print(chunks);
  Serial.println(" chunks");

  for (int i = 0; i < chunks; i++) {
    int start = i * chunkSize;
    int end = min(start + chunkSize, dataLen);
    String chunk = data.substring(start, end);
    
    longChar->setValue(chunk.c_str());
    longChar->notify();
    
    Serial.print("  Chunk ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(chunks);
    Serial.print(": ");
    Serial.println(chunk);
    
    delay(50); // Delay nhỏ giữa các gói
  }
  
  // Gửi tín hiệu kết thúc
  longChar->setValue("END");
  longChar->notify();
  Serial.println("✅ Long string sent!");
}

// ====================================
// SETUP
// ====================================
void setup() {
  Serial.begin(115200);
  Serial.println("🚀 BLE Server Advanced starting...");

  // Khởi tạo BLE
  BLEDevice::init("ESP32_Secure");
  
  // Tạo server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Tạo service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 1. Characteristic NOTIFY (dữ liệu thay đổi)
  notifyChar = pService->createCharacteristic(
    CHAR_NOTIFY_UUID,
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY
  );
  notifyChar->addDescriptor(new BLE2902()); // Descriptor cho notify

  // 2. Characteristic LONG STRING (chuỗi dài)
  longChar = pService->createCharacteristic(
    CHAR_LONG_UUID,
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY
  );
  longChar->addDescriptor(new BLE2902());

  // Khởi động service
  pService->start();

  // ====================================
  // CẤU HÌNH BẢO MẬT (Secure Pairing)
  // ====================================
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setCapability(ESP_IO_CAP_OUT); // Hiển thị passkey
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new MySecurity());

  // Bắt đầu quảng bá
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("📡 Broadcasting with security...");
  Serial.println("🔑 Passkey: 123456");
}

// ====================================
// LOOP
// ====================================
void loop() {
  if (deviceConnected) {
    // 1. GỬI NOTIFY (nhiệt độ thay đổi)
    static int counter = 0;
    float temp = 20.0 + (rand() % 100) / 10.0;
    String tempStr = String(temp, 1) + "C";
    
    notifyChar->setValue(tempStr.c_str());
    notifyChar->notify();
    
    Serial.print("📊 Notify #");
    Serial.print(counter++);
    Serial.print(": ");
    Serial.println(tempStr);

    // 2. GỬI CHUỖI DÀI (mỗi 10 giây)
    if (counter % 5 == 0) {
      String longData = "This is a very long string that needs to be split into multiple BLE packets because the MTU size is limited to 20-23 bytes per transmission. ESP32 BLE can handle this automatically but we're demonstrating manual splitting for educational purposes!";
      sendLongString(longData);
    }

    delay(2000);
  } else {
    Serial.println("⏳ Waiting for connection...");
    delay(3000);
  }
}