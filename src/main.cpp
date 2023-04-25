#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <BLEPeripheral.h>

// MPU-6050 Setup
MPU6050 mpu;

// BLE Setup
BLEPeripheral blePeripheral = BLEPeripheral();
BLEService sensorService = BLEService("1101"); // Custom service UUID
BLECharacteristic sensorCharacteristic = BLECharacteristic("2101", BLENotify, 12); // Custom characteristic UUID, 12 bytes for X, Y, Z, and Resultant g-force values (3 bytes each)

void setup() {
  // MPU-6050 Setup
  Wire.begin();
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  mpu.setDLPFMode(MPU6050_DLPF_5HZ);
  mpu.setSampleRateDivider(19);

  // BLE Setup
  blePeripheral.setLocalName("HeadImpactDetector");
  blePeripheral.setAdvertisedServiceUuid(sensorService.uuid());
  blePeripheral.addAttribute(sensorService);
  blePeripheral.addAttribute(sensorCharacteristic);
  blePeripheral.begin();
}

void loop() {
  // Read MPU-6050 data
  float ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Calculate g-force values
  float ax_g = ax / 16384.0 * 9.81;
  float ay_g = ay / 16384.0 * 9.81;
  float az_g = az / 16384.0 * 9.81;
  float resultant_g = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);

  // Prepare data for BLE transmission
  int16_t bleData[4] = {
    static_cast<int16_t>(ax_g * 100),
    static_cast<int16_t>(ay_g * 100),
    static_cast<int16_t>(az_g * 100),
    static_cast<int16_t>(resultant_g * 100)
  };

  // Update BLE characteristic with the new data
  sensorCharacteristic.setValue((unsigned char*)bleData, sizeof(bleData));

}
