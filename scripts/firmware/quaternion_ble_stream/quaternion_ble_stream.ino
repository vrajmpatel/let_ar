// SPDX-FileCopyrightText: 2024
// SPDX-License-Identifier: MIT

/*
 * QUATERNION + MAGNETOMETER + LINEAR ACCELERATION BLE STREAMER with BNO085 IMU
 * for Adafruit LED Glasses Driver (nRF52840)
 * 
 * This sketch reads quaternion, magnetometer, and linear acceleration data from 
 * a BNO085 9-DoF IMU via I2C and streams it over BLE UART at the maximum possible rate.
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
 * 
 * Magnetometer Packet Format (16 bytes):
 *   Byte 0:     '!'  (start marker)
 *   Byte 1:     'M'  (magnetometer identifier)
 *   Bytes 2-5:  mag_x (float, 4 bytes) - micro Tesla (uT)
 *   Bytes 6-9:  mag_y (float, 4 bytes) - micro Tesla (uT)
 *   Bytes 10-13: mag_z (float, 4 bytes) - micro Tesla (uT)
 *   Byte 14:    checksum (~sum of bytes 0-13)
 *   Byte 15:    newline '\n'
 * 
 * Linear Acceleration Packet Format (16 bytes):
 *   Byte 0:     '!'  (start marker)
 *   Byte 1:     'A'  (linear acceleration identifier)
 *   Bytes 2-5:  accel_x (float, 4 bytes) - m/s^2
 *   Bytes 6-9:  accel_y (float, 4 bytes) - m/s^2
 *   Bytes 10-13: accel_z (float, 4 bytes) - m/s^2
 *   Byte 14:    checksum (~sum of bytes 0-13)
 *   Byte 15:    newline '\n'
 * 
 * Magnetometer Units:
 *   The BNO085 SH2_MAGNETIC_FIELD_CALIBRATED report provides calibrated
 *   magnetic field readings in micro Tesla (uT). No conversion needed.
 *   Citation: Adafruit BNO085 datasheet, Page 31:
 *   "Magnetic Field Strength Vector / Magnetometer: Three axes of magnetic 
 *   field sensing in micro Teslas (uT)"
 * 
 * Linear Acceleration Units:
 *   The BNO085 SH2_LINEAR_ACCELERATION report provides acceleration with
 *   gravity removed, in m/s^2. This is useful for detecting device motion
 *   without the constant 9.8 m/s^2 gravity component.
 *   Citation: Adafruit BNO085 datasheet, Page 6 & 31:
 *   "Linear Acceleration Vector: Three axes of linear acceleration data 
 *   (acceleration minus gravity) in m/s^2"
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
bool newQuatData = false;

// Magnetometer data (in micro Tesla, uT)
// Citation: BNO085 datasheet - "Three axes of magnetic field sensing in micro Teslas (uT)"
float mag_x = 0.0f;
float mag_y = 0.0f;
float mag_z = 0.0f;
bool newMagData = false;

// Linear acceleration data (in m/s^2, gravity removed)
// Citation: BNO085 datasheet - "Linear Acceleration Vector: Three axes of linear 
// acceleration data (acceleration minus gravity) in m/s^2"
float linaccel_x = 0.0f;
float linaccel_y = 0.0f;
float linaccel_z = 0.0f;
bool newLinAccelData = false;

// Statistics
uint32_t packetCount = 0;
uint32_t magPacketCount = 0;
uint32_t linAccelPacketCount = 0;
uint32_t lastStatsTime = 0;
uint32_t imuReadCount = 0;
uint32_t magReadCount = 0;
uint32_t linAccelReadCount = 0;

// Packet buffers
uint8_t quatPacket[20];
uint8_t magPacket[16];
uint8_t linAccelPacket[16];

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
  Serial.println("Setting sensor reports...");
  
  // Enable rotation vector (quaternion) at fastest rate
  // Citation: BNO085 datasheet - "Absolute Orientation / Rotation Vector: 
  // Four-point quaternion output for accurate data manipulation"
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR, REPORT_INTERVAL_US)) {
    Serial.println("Could not enable rotation vector report");
  } else {
    Serial.print("Rotation vector enabled at ");
    Serial.print(1000000 / REPORT_INTERVAL_US);
    Serial.println(" Hz");
  }
  
  // Enable calibrated magnetometer report
  // Citation: BNO085 datasheet, Page 31 - "Magnetic Field Strength Vector / Magnetometer:
  // Three axes of magnetic field sensing in micro Teslas (uT)"
  // The SH2_MAGNETIC_FIELD_CALIBRATED report provides pre-calibrated readings
  // already in physical units (uT) - no LSB conversion needed.
  if (!bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED, REPORT_INTERVAL_US)) {
    Serial.println("Could not enable magnetometer report");
  } else {
    Serial.print("Magnetometer enabled at ");
    Serial.print(1000000 / REPORT_INTERVAL_US);
    Serial.println(" Hz");
  }
  
  // Enable linear acceleration report (gravity removed)
  // Citation: BNO085 datasheet, Page 6 & 31 - "Linear Acceleration Vector:
  // Three axes of linear acceleration data (acceleration minus gravity) in m/s^2"
  // Report ID: SH2_LINEAR_ACCELERATION (0x04)
  // This report uses sensor fusion to subtract the gravity vector from raw
  // accelerometer readings, providing pure motion acceleration.
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION, REPORT_INTERVAL_US)) {
    Serial.println("Could not enable linear acceleration report");
  } else {
    Serial.print("Linear acceleration enabled at ");
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
// PACKET BUILDING
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

/*
 * Build magnetometer packet (16 bytes)
 * Values are in micro Tesla (uT) - already calibrated by BNO085
 * Citation: Adafruit BNO085 datasheet, Page 31
 */
void buildMagnetometerPacket(float mx, float my, float mz) {
  uint8_t checksum = 0;
  
  // Start marker
  magPacket[0] = '!';
  checksum += '!';
  
  // Magnetometer identifier
  magPacket[1] = 'M';
  checksum += 'M';
  
  // Copy float values (little-endian) - units: micro Tesla (uT)
  memcpy(&magPacket[2], &mx, 4);
  memcpy(&magPacket[6], &my, 4);
  memcpy(&magPacket[10], &mz, 4);
  
  // Calculate checksum over float bytes
  for (int i = 2; i < 14; i++) {
    checksum += magPacket[i];
  }
  
  // Checksum (inverted sum)
  magPacket[14] = ~checksum;
  
  // Newline terminator
  magPacket[15] = '\n';
}

/*
 * Build linear acceleration packet (16 bytes)
 * Values are in m/s^2 - gravity has been removed by BNO085 sensor fusion
 * Citation: Adafruit BNO085 datasheet, Page 6 & 31:
 *   "Linear Acceleration Vector: Three axes of linear acceleration data
 *   (acceleration minus gravity) in m/s^2"
 */
void buildLinearAccelPacket(float ax, float ay, float az) {
  uint8_t checksum = 0;
  
  // Start marker
  linAccelPacket[0] = '!';
  checksum += '!';
  
  // Linear acceleration identifier ('A' for Acceleration)
  linAccelPacket[1] = 'A';
  checksum += 'A';
  
  // Copy float values (little-endian) - units: m/s^2
  memcpy(&linAccelPacket[2], &ax, 4);
  memcpy(&linAccelPacket[6], &ay, 4);
  memcpy(&linAccelPacket[10], &az, 4);
  
  // Calculate checksum over float bytes
  for (int i = 2; i < 14; i++) {
    checksum += linAccelPacket[i];
  }
  
  // Checksum (inverted sum)
  linAccelPacket[14] = ~checksum;
  
  // Newline terminator
  linAccelPacket[15] = '\n';
}

// ============================================================================
// BLE CALLBACKS
// ============================================================================

void connect_callback(uint16_t conn_handle) {
  (void)conn_handle;
  isConnected = true;
  packetCount = 0;
  magPacketCount = 0;
  linAccelPacketCount = 0;
  lastStatsTime = millis();
  
  Serial.println("BLE Connected!");
  Serial.println("Starting quaternion + magnetometer + linear acceleration stream...");
  
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
 * Read sensor data from BNO085. 
 * Handles quaternion, magnetometer, and linear acceleration reports.
 * Returns true if any new data is available.
 */
bool readSensors() {
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
      // Quaternion from sensor fusion
      quat_w = sensorValue.un.rotationVector.real;
      quat_x = sensorValue.un.rotationVector.i;
      quat_y = sensorValue.un.rotationVector.j;
      quat_z = sensorValue.un.rotationVector.k;
      imuReadCount++;
      newQuatData = true;
      return true;
      
    case SH2_MAGNETIC_FIELD_CALIBRATED:
      // Calibrated magnetometer readings in micro Tesla (uT)
      // Citation: BNO085 datasheet - values are pre-calibrated, no LSB conversion needed
      // "Three axes of magnetic field sensing in micro Teslas (uT)"
      mag_x = sensorValue.un.magneticField.x;
      mag_y = sensorValue.un.magneticField.y;
      mag_z = sensorValue.un.magneticField.z;
      magReadCount++;
      newMagData = true;
      return true;
      
    case SH2_LINEAR_ACCELERATION:
      // Linear acceleration in m/s^2 (gravity removed)
      // Citation: BNO085 datasheet, Page 6 & 31:
      // "Linear Acceleration Vector: Three axes of linear acceleration data
      // (acceleration minus gravity) in m/s^2"
      // The sensor fusion algorithm subtracts the gravity vector from raw
      // accelerometer readings, leaving only motion-induced acceleration.
      linaccel_x = sensorValue.un.linearAcceleration.x;
      linaccel_y = sensorValue.un.linearAcceleration.y;
      linaccel_z = sensorValue.un.linearAcceleration.z;
      linAccelReadCount++;
      newLinAccelData = true;
      return true;
      
    default:
      break;
  }
  
  return false;
}

// ============================================================================
// DATA STREAMING
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

/*
 * Stream magnetometer data over BLE.
 * Values are in micro Tesla (uT) - already in physical units.
 * Returns true if packet was sent successfully.
 */
bool streamMagnetometer() {
  if (!isConnected) {
    return false;
  }
  
  // Build packet with current magnetometer values (uT)
  buildMagnetometerPacket(mag_x, mag_y, mag_z);
  
  // Send via BLE UART
  uint16_t written = bleuart.write(magPacket, 16);
  
  if (written == 16) {
    magPacketCount++;
    return true;
  }
  
  return false;
}

/*
 * Stream linear acceleration data over BLE.
 * Values are in m/s^2 - gravity has been removed.
 * Citation: BNO085 datasheet - "acceleration minus gravity"
 * Returns true if packet was sent successfully.
 */
bool streamLinearAccel() {
  if (!isConnected) {
    return false;
  }
  
  // Build packet with current linear acceleration values (m/s^2)
  buildLinearAccelPacket(linaccel_x, linaccel_y, linaccel_z);
  
  // Send via BLE UART
  uint16_t written = bleuart.write(linAccelPacket, 16);
  
  if (written == 16) {
    linAccelPacketCount++;
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
    Serial.print("Quat reads/sec: ");
    Serial.print(imuReadCount);
    Serial.print(" | Mag reads/sec: ");
    Serial.print(magReadCount);
    Serial.print(" | LinAccel reads/sec: ");
    Serial.print(linAccelReadCount);
    
    if (isConnected) {
      Serial.print(" | BLE Quat: ");
      Serial.print(packetCount);
      Serial.print(" | BLE Mag: ");
      Serial.print(magPacketCount);
      Serial.print(" | BLE Accel: ");
      Serial.print(linAccelPacketCount);
      Serial.print(" (");
      Serial.print(packetCount * 20 + magPacketCount * 16 + linAccelPacketCount * 16);
      Serial.print(" bytes/sec)");
    }
    
    Serial.println();
    Serial.print("  Quat: w=");
    Serial.print(quat_w, 3);
    Serial.print(" x=");
    Serial.print(quat_x, 3);
    Serial.print(" y=");
    Serial.print(quat_y, 3);
    Serial.print(" z=");
    Serial.println(quat_z, 3);
    
    // Magnetometer values in micro Tesla (uT)
    Serial.print("  Mag (uT): x=");
    Serial.print(mag_x, 2);
    Serial.print(" y=");
    Serial.print(mag_y, 2);
    Serial.print(" z=");
    Serial.println(mag_z, 2);
    
    // Linear acceleration values in m/s^2
    Serial.print("  LinAccel (m/s2): x=");
    Serial.print(linaccel_x, 2);
    Serial.print(" y=");
    Serial.print(linaccel_y, 2);
    Serial.print(" z=");
    Serial.println(linaccel_z, 2);
    
    packetCount = 0;
    magPacketCount = 0;
    linAccelPacketCount = 0;
    imuReadCount = 0;
    magReadCount = 0;
    linAccelReadCount = 0;
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
  Serial.println("=============================================");
  Serial.println("  Quaternion + Mag + LinAccel BLE Streamer");
  Serial.println("  BNO085 IMU - Maximum Speed Transmission");
  Serial.println("=============================================");
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
  // Always try to read sensors (even when not connected)
  readSensors();
  
  // Stream over BLE when connected and we have new data
  if (isConnected) {
    // Send quaternion data if available
    if (newQuatData) {
      if (streamQuaternion()) {
        ledToggle();
      }
      newQuatData = false;
    }
    
    // Send magnetometer data if available
    if (newMagData) {
      streamMagnetometer();
      newMagData = false;
    }
    
    // Send linear acceleration data if available
    if (newLinAccelData) {
      streamLinearAccel();
      newLinAccelData = false;
    }
  } else {
    // Slow blink while waiting for connection
    static uint32_t lastBlink = 0;
    if (millis() - lastBlink > 500) {
      ledToggle();
      lastBlink = millis();
    }
    // Clear flags when not connected
    newQuatData = false;
    newMagData = false;
    newLinAccelData = false;
  }
  
  // Print stats every second
  printStats();
}
