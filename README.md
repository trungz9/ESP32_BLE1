# ESP32_BLE1

# 📘 BÀI TẬP BLE ESP32 - HỆ THỐNG HOÀN CHỈNH

> Bài tập thực hành BLE (Bluetooth Low Energy) với ESP32  
> **Sinh viên:** [Tên của bạn]  
> **Lớp:** [Lớp của bạn]  
> **Giảng viên:** [Tên thầy]

---

## 📋 MỤC LỤC

1. [BLE Peripheral (GATT Server)](#1-ble-peripheral-gatt-server)
2. [BLE Central (GATT Client)](#2-ble-central-gatt-client)
3. [BLE 2 ESP32 (Client ↔ Server)](#3-ble-2-esp32-client--server)
4. [BLE Nâng Cao](#4-ble-nâng-cao)
5. [Hardware & Cảm Biến](#5-hardware--cảm-biến)
6. [Hướng Dẫn Sử Dụng](#6-hướng-dẫn-sử-dụng)

---

## 1. BLE PERIPHERAL (GATT SERVER)

### 🎯 Mục tiêu
- ESP32 hoạt động như BLE Server
- Điều khiển 3 LED qua BLE từ điện thoại
- Đọc trạng thái LED qua READ characteristic
- Ghi lệnh điều khiển qua WRITE characteristic

### 📁 File code
- [`01_ble_peripheral/ble_peripheral.ino`](./01_ble_peripheral/ble_peripheral.ino)

### 🔌 Sơ đồ kết nối
```
ESP32          LED
GPIO 4    -->  LED1 --> GND
GPIO 16   -->  LED2 --> GND
GPIO 17   -->  LED3 --> GND
```

### 📱 Cách kiểm tra
1. Upload code vào ESP32
2. Mở app BLE Scanner trên điện thoại
3. Tìm thiết bị **"ESP32_BLE"**
4. Kết nối và tìm service UUID: `12345678-1234-1234-1234-1234567890ab`
5. **Đọc** characteristic `...AC` → xem trạng thái LED (vd: "0,0,1")
6. **Ghi** characteristic `...AD` → gửi lệnh:
   - Ghi `1` → Toggle LED1
   - Ghi `2` → Toggle LED2
   - Ghi `3` → Toggle LED3

### 🎥 Demo
![Demo Peripheral](./images/demo_peripheral.gif)

---

## 2. BLE CENTRAL (GATT CLIENT)

### 🎯 Mục tiêu
- ESP32 quét các thiết bị BLE xung quanh
- Kết nối đến 1 thiết bị BLE (ESP32 Peripheral)
- Đọc/ghi characteristic từ xa

### 📁 File code
- [`02_ble_central/ble_central.ino`](./02_ble_central/ble_central.ino)

### 🔄 Luồng hoạt động
```
ESP32 Central
    ↓
Quét BLE (5s)
    ↓
Tìm "ESP32_BLE"
    ↓
Kết nối
    ↓
Đọc trạng thái LED
    ↓
Gửi lệnh điều khiển
```

### 📊 Kết quả Serial Monitor
```
BLE Central starting...
Found device: ESP32_BLE
-> Target device found!
Connecting to xx:xx:xx:xx:xx:xx
Connected!
LED Status: 0,0,0
Sent command: 1
LED Status: 1,0,0
```

---

## 3. BLE 2 ESP32 (CLIENT ↔ SERVER)

### 🎯 Mục tiêu
- **ESP32 #1** (Peripheral): Phát dữ liệu cảm biến nhiệt độ
- **ESP32 #2** (Central): Nhận và hiển thị dữ liệu

### 📁 File code
- [`03_ble_2_esp32/ble_peripheral.ino`](./03_ble_2_esp32/ble_peripheral.ino) - Server
- [`03_ble_2_esp32/ble_central.ino`](./03_ble_2_esp32/ble_central.ino) - Client

### 🌡️ Dữ liệu truyền
- **Nhiệt độ giả lập**: 20.0°C - 30.0°C
- **Tần suất cập nhật**: 2 giây/lần
- **Phương thức**: BLE Notify

### 📊 Kết quả
**ESP32 Server:**
```
BLE Peripheral ready!
Temperature: 23.5C
Temperature: 26.2C
Temperature: 24.8C
```

**ESP32 Client:**
```
BLE Central starting...
-> Target found!
✅ Connected!
📊 Temperature: 23.5C
📊 Temperature: 26.2C
📊 Temperature: 24.8C
```

---

## 4. BLE NÂNG CAO

### 🎯 Mục tiêu
✅ **Notify**: Server gửi thông báo tự động khi dữ liệu thay đổi  
✅ **Secure Pairing**: Kết nối có mã PIN bảo mật  
✅ **Long String**: Truyền chuỗi dài (split thành nhiều gói)

### 📁 File code
- [`04_ble_advanced/ble_server_advanced.ino`](./04_ble_advanced/ble_server_advanced.ino)
- [`04_ble_advanced/ble_client_advanced.ino`](./04_ble_advanced/ble_client_advanced.ino)

### 🔐 Tính năng bảo mật
- **Authentication**: ESP_LE_AUTH_REQ_SC_MITM_BOND
- **Passkey**: 123456
- **Encryption**: ESP_BLE_SEC_ENCRYPT

### 📦 Xử lý chuỗi dài
- **MTU size**: 23 bytes
- **Payload**: 20 bytes/gói (trừ 3 bytes header)
- **Phương pháp**: Split → Send → Reassemble

### 📊 Kết quả truyền chuỗi dài

**Server:**
```
📤 Sending long string...
Total length: 245 bytes, 13 chunks
  Chunk 1/13: This is a very lon
  Chunk 2/13: g string that need
  Chunk 3/13: s to be split into
  ...
  Chunk 13/13: al purposes!
✅ Long string sent!
```

**Client:**
```
📦 Chunk received: This is a very lon
📦 Chunk received: g string that need
...
📥 Complete message:
This is a very long string that needs to be split into multiple BLE packets because the MTU size is limited to 20-23 bytes per transmission...
Total length: 245 bytes
```

---

## 5. HARDWARE & CẢM BIẾN

### 🔧 Linh kiện sử dụng

| Linh kiện | Số lượng | Mục đích |
|-----------|----------|----------|
| ESP32 Dev Board | 2 | Server & Client |
| LED | 3 | Kiểm tra điều khiển |
| Resistor 220Ω | 3 | Hạn dòng LED |
| AHT20 + BMP280 | 1 | Cảm biến nhiệt độ, độ ẩm, áp suất |
| Breadboard | 1 | Kết nối mạch |
| Dây nối | - | Kết nối linh kiện |

### 🌡️ Module cảm biến AHT20 + BMP280

**Thông số:**
- **AHT20**: Nhiệt độ (-40°C đến 85°C), Độ ẩm (0-100% RH)
- **BMP280**: Áp suất (300-1100 hPa), Nhiệt độ
- **Giao tiếp**: I2C (SCL, SDA)
- **Nguồn**: 3.3V hoặc 5V (có LDO onboard)

**Địa chỉ I2C:**
- AHT20: `0x38`
- BMP280: `0x76` hoặc `0x77`

**Kết nối với ESP32:**
```
Module        ESP32
VIN/VDD  -->  3V3
GND      -->  GND
SCL      -->  GPIO 22 (SCL)
SDA      -->  GPIO 21 (SDA)
```

**Lưu ý:**
- ✅ Nối VIN với 3V3 của ESP32 (an toàn)
- ✅ Module có LDO nên hỗ trợ cả 3.3V và 5V
- ⚠️ ESP32 Dev Board thường chỉ có chân 3V3 ra ngoài

---

## 6. HƯỚNG DẪN SỬ DỤNG

### 📥 Cài đặt

#### 1. Cài đặt Arduino IDE
- Tải về: https://www.arduino.cc/en/software
- Phiên bản khuyến nghị: 2.0 trở lên

#### 2. Cài đặt ESP32 Board
```
File → Preferences → Additional Board Manager URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Tools → Board → Boards Manager → Tìm "ESP32" → Install
```

#### 3. Chọn Board
```
Tools → Board → ESP32 Arduino → ESP32 Dev Module
```

#### 4. Cấu hình Upload
```
Upload Speed: 115200
Flash Frequency: 80MHz
Partition Scheme: Default
```

### 🚀 Upload code

#### Lỗi thường gặp: `Failed to connect to ESP32`

**Giải pháp:**
1. **Giữ nút BOOT** khi upload
2. Thay cáp USB (cáp phải hỗ trợ truyền data)
3. Giảm Upload Speed xuống 115200
4. Kiểm tra COM Port đúng
5. Đóng Serial Monitor trước khi upload

### 🔍 Kiểm tra hoạt động

#### Bước 1: Mở Serial Monitor
```
Tools → Serial Monitor
Baud rate: 115200
```

#### Bước 2: Quan sát log
- ✅ Thấy "BLE starting..." → Code chạy OK
- ✅ Thấy "Connected!" → Kết nối thành công
- ❌ Không thấy gì → Kiểm tra baud rate

#### Bước 3: Test với điện thoại
**App khuyến nghị:**
- Android: **nRF Connect** (Nordic Semiconductor)
- iOS: **LightBlue** (Punch Through)

---

## 🛠️ TROUBLESHOOTING

### Vấn đề 1: ESP32 bị nóng
**Nguyên nhân:**
- WiFi/BLE hoạt động liên tục
- Nguồn 5V qua VIN (LDO phải hạ áp)

**Giải pháp:**
```cpp
// Thêm delay trong loop
void loop() {
  // Code của bạn
  delay(100); // Cho CPU nghỉ
}
```

### Vấn đề 2: Không quét thấy thiết bị BLE
**Kiểm tra:**
- ✅ ESP32 Server đã upload code và đang chạy?
- ✅ Serial Monitor của Server thấy "Broadcasting..."?
- ✅ Tên thiết bị trong code Client khớp với Server?
- ✅ UUID của Service/Characteristic khớp nhau?

### Vấn đề 3: Kết nối BLE bị ngắt
**Nguyên nhân:**
- Khoảng cách quá xa (>10m)
- Nhiễu sóng WiFi/BLE
- Nguồn ESP32 không ổn định

**Giải pháp:**
- Đặt 2 ESP32 gần nhau (<5m)
- Tắt WiFi nếu không dùng
- Dùng nguồn 5V/2A ổn định

---

## 📚 TÀI LIỆU THAM KHẢO

### Official Documentation
- [ESP32 BLE Arduino](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
- [Bluetooth LE Specifications](https://www.bluetooth.com/specifications/specs/)

### UUID Generator
- [Online UUID Generator](https://www.uuidgenerator.net/)

### BLE Tutorials
- [ESP32 BLE Server-Client](https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/)
- [BLE Notify Tutorial](https://www.electronicshub.org/esp32-ble-tutorial/)

---
