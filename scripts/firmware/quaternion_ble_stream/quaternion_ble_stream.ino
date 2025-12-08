// SPDX-FileCopyrightText: 2024
// SPDX-License-Identifier: MIT

/*
 * QUATERNION BLE STREAMER with BNO085 IMU
 * for Adafruit LED Glasses Driver (nRF52840)
 * 
 * This sketch reads quaternion data from a BNO085 9-DoF IMU via I2C
 * and streams it over BLE UART at the maximum possible rate.
 * The onboard LED blinks on each transmission.
 * 
 * Hardware:
 *   - Adafruit LED Glasses Driver - nRF52840 BLE
 *   - Adafruit BNO085 9-DoF IMU (I2C connection)
 * 
 * Libraries Required:
 *   - Adafruit_nRF52_Arduino (core)
 *   - Adafruit_BNO08x
 *   - Adafruit_BusIO
 * 
 * I2C Wiring (BNO085 to nRF52840):
 *   - VIN -> 3.3V
 *   - GND -> GND
 *   - SCL -> SCL pin
 *   - SDA -> SDA pin
 * 
 * Quaternion Packet Format (20 bytes):
 *   Byte 0:     '!'  (start marker)
 *   Byte 1:     'Q'  (quaternion identifier)
 *   Bytes 2-5:  w (float, 4 bytes)
 *   Bytes 6-9:  x (float, 4 bytes)
 *   Bytes 10-13: y (float, 4 bytes)
 *   Bytes 14-17: z (float, 4 bytes)
 *   Byte 18:    checksum (~sum of bytes 0-17)
 *   Byte 19:    newline '\n'
 */

#include <Wire.h>
#include <bluefruit.h>
#include <Adafruit_BNO08x.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// LED Configuration - Adafruit LED Glasses Driver uses P0.31, active HIGH
#define LED_PIN           31
#define LED_ACTIVE_HIGH   true

// BLE Configuration
#define DEVICE_NAME       "QuatStream"
#define TX_POWER          4               // dBm (-40 to +8)

// BNO085 Configuration
#define BNO085_I2C_ADDR   0x4A            // Default I2C address (0x4B if DI pin high)
#define BNO085_RESET_PIN  -1              // Set to GPIO if using hardware reset

// Report Configuration - rotation_vector gives best quaternion fusion
#define REPORT_INTERVAL_US 5000           // 5ms = 200Hz (fastest stable rate)

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// BNO085 IMU
Adafruit_BNO08x bno08x(BNO085_RESET_PIN);
sh2_SensorValue_t sensorValue;
bool imuReady = false;

// BLE
BLEUart bleuart;
bool isConnected = false;

// Quaternion data
float quat_w = 1.0f;
float quat_x = 0.0f;
float quat_y = 0.0f;
float quat_z = 0.0f;

// Statistics
uint32_t packetCount = 0;
uint32_t lastStatsTime = 0;
uint32_t imuReadCount = 0;

// Packet buffer
uint8_t quatPacket[20];

// ============================================================================
// LED CONTROL
// ============================================================================

void ledOn() {
  digitalWrite(LED_PIN, LED_ACTIVE_HIGH ? HIGH : LOW);
}

void ledOff() {
  digitalWrite(LED_PIN, LED_ACTIVE_HIGH ? LOW : HIGH);
}

void ledToggle() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void ledBlink(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    ledOn();
    delay(delayMs);
    ledOff();
    delay(delayMs);
  }
}

// ============================================================================
// BNO085 IMU SETUP
// ============================================================================

void setReports() {
  Serial.println("Setting rotation vector report...");
  
  // Enable rotation vector (quaternion) at fastest rate
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR, REPORT_INTERVAL_US)) {
    Serial.println("Could not enable rotation vector report");
  } else {
    Serial.print("Rotation vector enabled at ");
    Serial.print(1000000 / REPORT_INTERVAL_US);
    Serial.println(" Hz");
  }
}

bool setupIMU() {
  Serial.println("Initializing BNO085 IMU...");
  
  // Initialize I2C
  Wire.begin();
  
  // Try to initialize the BNO085
  if (!bno08x.begin_I2C(BNO085_I2C_ADDR, &Wire)) {
    Serial.println("Failed to find BNO085 on I2C!");
    Serial.print("Tried address: 0x");
    Serial.println(BNO085_I2C_ADDR, HEX);
    return false;
  }
  
  Serial.println("BNO085 found!");
  
  // Print product info
  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version ");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.println(bno08x.prodIds.entry[n].swVersionPatch);
  }
  
  // Enable reports
  setReports();
  
  return true;
}

// ============================================================================
// QUATERNION PACKET BUILDING
// ============================================================================

void buildQuaternionPacket(float w, float x, float y, float z) {
  uint8_t checksum = 0;
  
  // Start marker
  quatPacket[0] = '!';
  checksum += '!';
  
  // Quaternion identifier
  quatPacket[1] = 'Q';
  checksum += 'Q';
  
  // Copy float values (little-endian)
  memcpy(&quatPacket[2], &w, 4);
  memcpy(&quatPacket[6], &x, 4);
  memcpy(&quatPacket[10], &y, 4);
  memcpy(&quatPacket[14], &z, 4);
  
  // Calculate checksum over float bytes
  for (int i = 2; i < 18; i++) {
    checksum += quatPacket[i];
  }
  
  // Checksum (inverted sum)
  quatPacket[18] = ~checksum;
  
  // Newline terminator
  quatPacket[19] = '\n';
}

// ============================================================================
// BLE CALLBACKS
// ============================================================================

void connect_callback(uint16_t conn_handle) {
  (void)conn_handle;
  isConnected = true;
  packetCount = 0;
  lastStatsTime = millis();
  
  Serial.println("BLE Connected!");
  Serial.println("Starting quaternion stream...");
  
  // Rapid blink to indicate connection
  ledBlink(5, 50);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void)conn_handle;
  (void)reason;
  isConnected = false;
  
  Serial.println("BLE Disconnected");
  Serial.print("Reason: 0x");
  Serial.println(reason, HEX);
}

// ============================================================================
// BLE SETUP
// ============================================================================

void setupBLE() {
  Serial.println("Initializing BLE...");
  
  // Initialize Bluefruit
  Bluefruit.begin();
  Bluefruit.setTxPower(TX_POWER);
  Bluefruit.setName(DEVICE_NAME);
  
  // Set connection callbacks
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  
  // Configure for maximum throughput
  Bluefruit.Periph.setConnIntervalMS(7.5, 15);  // Fastest allowed interval
  
  // Start BLE UART service
  bleuart.begin();
  
  // Setup advertising
  startAdvertising();
  
  Serial.println("BLE initialized");
}

void startAdvertising() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  
  // Add device name in scan response
  Bluefruit.ScanResponse.addName();
  
  // Advertising configuration
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);   // Fast: 20ms, Slow: 152.5ms
  Bluefruit.Advertising.setFastTimeout(30);     // 30 seconds in fast mode
  Bluefruit.Advertising.start(0);               // Advertise forever
  
  Serial.println("Advertising started");
}

// ============================================================================
// IMU READING
// ============================================================================

/*
 * Read quaternion from BNO085. Returns true if new data available.
 */
bool readQuaternion() {
  if (!imuReady) {
    return false;
  }
  
  if (bno08x.wasReset()) {
    Serial.println("BNO085 was reset, re-enabling reports...");
    setReports();
  }
  
  if (!bno08x.getSensorEvent(&sensorValue)) {
    return false;
  }
  
  switch (sensorValue.sensorId) {
    case SH2_ROTATION_VECTOR:
      quat_w = sensorValue.un.rotationVector.real;
      quat_x = sensorValue.un.rotationVector.i;
      quat_y = sensorValue.un.rotationVector.j;
      quat_z = sensorValue.un.rotationVector.k;
      imuReadCount++;
      return true;
      
    default:
      break;
  }
  
  return false;
}

// ============================================================================
// QUATERNION STREAMING
// ============================================================================

/*
 * Stream quaternion data over BLE.
 * Returns true if packet was sent successfully.
 */
bool streamQuaternion() {
  if (!isConnected) {
    return false;
  }
  
  // Build packet with current quaternion values
  buildQuaternionPacket(quat_w, quat_x, quat_y, quat_z);
  
  // Send via BLE UART
  uint16_t written = bleuart.write(quatPacket, 20);
  
  if (written == 20) {
    packetCount++;
    return true;
  }
  
  return false;
}

// ============================================================================
// STATISTICS
// ============================================================================

void printStats() {
  uint32_t now = millis();
  if (now - lastStatsTime >= 1000) {
    Serial.print("IMU reads/sec: ");
    Serial.print(imuReadCount);
    
    if (isConnected) {
      Serial.print(" | BLE packets/sec: ");
      Serial.print(packetCount);
      Serial.print(" (");
      Serial.print(packetCount * 20);
      Serial.print(" bytes/sec)");
    }
    
    Serial.print(" | Quat: w=");
    Serial.print(quat_w, 3);
    Serial.print(" x=");
    Serial.print(quat_x, 3);
    Serial.print(" y=");
    Serial.print(quat_y, 3);
    Serial.print(" z=");
    Serial.println(quat_z, 3);
    
    packetCount = 0;
    imuReadCount = 0;
    lastStatsTime = now;
  }
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  
  // Wait for serial (with timeout for standalone operation)
  uint32_t startWait = millis();
  while (!Serial && (millis() - startWait < 2000)) {
    delay(10);
  }
  
  Serial.println();
  Serial.println("==========================================");
  Serial.println("  Quaternion BLE Streamer + BNO085 IMU");
  Serial.println("  Maximum Speed Transmission");
  Serial.println("==========================================");
  Serial.println();
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  ledOff();
  
  // Initialize IMU
  imuReady = setupIMU();
  if (!imuReady) {
    Serial.println("WARNING: IMU not available, using fallback values");
    ledBlink(10, 100);  // Error indication
  } else {
    ledBlink(3, 100);   // Success indication
  }
  
  // Setup BLE
  setupBLE();
  
  Serial.println();
  Serial.println("Waiting for BLE connection...");
}

void loop() {
  // Always try to read IMU (even when not connected)
  bool newData = readQuaternion();
  
  // Stream over BLE when connected and we have new data
  if (isConnected) {
    if (newData) {
      if (streamQuaternion()) {
        // Toggle LED on each successful transmission
        ledToggle();
      }
    }
  } else {
    // Slow blink while waiting for connection
    static uint32_t lastBlink = 0;
    if (millis() - lastBlink > 500) {
      ledToggle();
      lastBlink = millis();
    }
  }
  
  // Print stats every second
  printStats();
}
