#include <Arduino.h>
// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>

#include <Wire.h>
#include <MPU6050.h>

// MPU-6050 Setup
MPU6050 mpu;

// BLE Setup
BLEPeripheral blePeripheral = BLEPeripheral();
BLEService sensorService = BLEService("1101"); // Custom service UUID
BLECharacteristic sensorCharacteristic = BLECharacteristic("2101", BLENotify, 12); // Custom characteristic UUID, 12 bytes for X, Y, Z, and Resultant g-force values (3 bytes each)

// Change the following line to use the other variant in .pio/libdeps/nrf52_dk/MPU6050/src/MPU6050.cpp
// See https://github.com/jrowberg/i2cdevlib/issues/457
#define dtostrf(Variable,Spaces,Precision ,S) (Variable)

void setup() {
  // MPU-6050 Setup
  Wire.begin();
  mpu.initialize();

  // BLE Setup
  blePeripheral.setLocalName("HeadImpactDetector");
  blePeripheral.setAdvertisedServiceUuid(sensorService.uuid());
  blePeripheral.addAttribute(sensorService);
  blePeripheral.addAttribute(sensorCharacteristic);
  blePeripheral.begin();
}

void loop() {
  // Read MPU-6050 data
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Prepare data for BLE transmission
  int16_t bleData[6] = { ax, ay, az, gx, gy, gz };

  // Update BLE characteristic with the new data
  sensorCharacteristic.setValue((unsigned char*)bleData, sizeof(bleData));

}
