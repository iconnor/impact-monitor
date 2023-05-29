#include <NimBLEDevice.h>
#include <NimBLEClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Button2.h>
#include <cstring>

#define BUTTON1_PIN 0 // GPIO0
#define BUTTON2_PIN 35 // GPIO35

TFT_eSPI tft = TFT_eSPI();
Button2 button1;           // Create an instance of the Button2 library
Button2 button2;           // Create an instance of the Button2 library

// Declare variables
float gravityX = 0;
float gravityY = 0;
float gravityZ = -1;
float lastX = 0;
float lastY = 0;
float lastZ = 0;
bool isAtRest = false;
float highScore = 0;
// Mass (in kg) and area (in m^2) for the hand or foot
float mass = 0.5;  // Estimate to use for mass
float area = 0.01; // Estimate to use for area
float pa_to_psi = 0.000145038; // Conversion factor 1 Pa = 0.000145038 PSI


// Set up BLE client
static NimBLEAdvertisedDevice* myDevice;
static NimBLEClient* myClient;

struct Vector3D {
    float x;
    float y;
    float z;
};

Vector3D removeGravity(float x, float y, float z) {
    Vector3D result;
    result.x = x - gravityX;
    result.y = y - gravityY;
    result.z = z - gravityZ;
    return result;
}

bool isDeviceAtRest(float x, float y, float z) {
    // For simplicity, here we return true if the magnitude is close to 1g.
    float magnitude = sqrt(x * x + y * y + z * z);
    return abs(magnitude - 1) < 0.35;
}
class MyClientCallback : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("Connected to Bluetooth device");
    }
    void onDisconnect(NimBLEClient* pClient) {
        Serial.println("Disconnected from Bluetooth device");
        myClient = nullptr;
    }
};

// Functions for handling gravity and sensor data
void updateGravity(float x, float y, float z) {
    if (isDeviceAtRest(x, y, z)) {
        Serial.println("Device is at rest: " + String(x) + ", " + String(y) + ", " + String(z));
        gravityX = x;
        gravityY = y;
        gravityZ = z;
    }
}


void updateHighScore(float number) {
  // Fullscreen black fill to clear the screen
  // Fullscreen black fill to clear the screen
  tft.fillScreen(TFT_BLACK);
  
  // set the circle's coordinates
  int16_t circleX = tft.width() / 2; 
  int16_t circleY = tft.height() / 2;

  // radius of the circle
  int16_t circleRadius = 50; // Adjust as necessary

  uint16_t color = TFT_WHITE;  // set the number's color
    // Add color class based on highScore value
    if (number < 1) {
        tft.fillCircle(circleX, circleY, circleRadius, TFT_WHITE);
        color = TFT_BLACK;
    } else if (number < 3) {
        tft.fillCircle(circleX, circleY, circleRadius, TFT_YELLOW);
        color = TFT_BLACK;
    } else if (number < 5) {
        tft.fillCircle(circleX, circleY, circleRadius, TFT_BLUE);
    } else {
        tft.fillCircle(circleX, circleY, circleRadius, TFT_RED);
    }
  
  // Convert the number to string to display it.
  char numberStr[6];  // enough to hold 0.00 to 10.00
  dtostrf(number, 4, 2, numberStr);  // convert float to string
  
  
  // Set the font size
  tft.setTextSize(2);

  // Draw the number in the middle of the circle
  tft.setTextColor(color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(numberStr, circleX, circleY);
}

// Button click event handler
void handleButton1Click(Button2& btn) {
  // Reset the highScore to 0
  highScore = 0;
  updateHighScore(highScore);
}

// Button 2 sets rest state
void handleButton2Click(Button2& btn) {
  // Reset the highScore to 0
  isAtRest = true;
  gravityX = lastX;
    gravityY = lastY;
    gravityZ = lastZ;
    highScore = 0;
  updateHighScore(highScore);
}

union {
    uint8_t b[4];
    float f;
} fconv;

// Callback for when characteristic value changes
void onNotify(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.println("Notification received with data length: " + String(length));
    if (length != 12) {
        Serial.println("Data length is not 12 bytes!");
        return;
    }  else {
        Serial.println("Data length is 12 bytes!" + String(length) + " data 0:" + String(pData[0]) + " 1:" + String(pData[1]) + " 2:" + String(pData[2]) + " 3:" + String(pData[3]) + " 4:" + String(pData[4]) + " 5:" + String(pData[5]) + " 6:" + String(pData[6]) + " 7:" + String(pData[7]) + " 8:" + String(pData[8]) + " 9:" + String(pData[9]) + " 10:" + String(pData[10]) + " 11:" + String(pData[11]));
    }

    // Assuming pCharacteristic is a NimBLERemoteCharacteristic object

    int16_t result = (pData[0] << 4) | pData[1];  // Perform byte swapping
    lastX = static_cast<float>(result) / 16384.0f;
    
    result = (pData[2] << 8) | pData[3];  // Perform byte swapping
    lastY = static_cast<float>(result) / 16384.0f;
    
    result = (pData[4] << 8) | pData[5];  // Perform byte swapping
    lastZ = static_cast<float>(result) / 16384.0f;

    updateGravity(lastX, lastY, lastZ);

    float accelerationX = removeGravity(lastX, lastY, lastZ).x;
    float accelerationY = removeGravity(lastX, lastY, lastZ).y;
    float accelerationZ = removeGravity(lastX, lastY, lastZ).z;

    Serial.println("Acceleration without gravity: " + String(accelerationX) + ", " + String(accelerationY) + ", " + String(accelerationZ));
    float magnitude = sqrt(accelerationX * accelerationX + accelerationY * accelerationY + accelerationZ * accelerationZ);

    Serial.println("Magnitude: " + String(magnitude));

    // Calculate force in Newtons (N)
    float force = mass * magnitude; 

    // Calculate pressure in Pascals (Pa)
    float pressure_pa = force / area;

    // Convert pressure to PSI
    float pressure_psi = pressure_pa * pa_to_psi;

    Serial.println("Force: " + String(force) + " N, Pressure: " + String(pressure_psi) + " PSI");


    // Call updateChart to reflect new data
    // updateChart();
    if (magnitude > highScore) {
      highScore = magnitude;
      updateHighScore(magnitude);
    }
}
// Initialize BLE and connect to device
void setup() {
    Serial.begin(115200);

    button1.begin(BUTTON1_PIN);  // Set the button pin number
    button2.begin(BUTTON2_PIN);  // Set the button pin number
    button1.setClickHandler(handleButton1Click);
    button2.setClickHandler(handleButton2Click);

    tft.init();
    tft.setSwapBytes(true);
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.println("Punch-o-meter");

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Set Bluetooth power

    // Begin scan for Bluetooth devices
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);
    pScan->start(5, false);

    tft.setTextSize(2);
    tft.println("Connecting...");
    // Wait for scan to complete
    while (myDevice == nullptr) {
        delay(1000);
        NimBLEScanResults foundDevices = pScan->getResults();
        for (int i = 0; i < foundDevices.getCount(); i++) {
            myDevice = new NimBLEAdvertisedDevice(foundDevices.getDevice(i));
            if (myDevice->getName() == "SENSOR_PRO") {
                break;
            } else {
                myDevice = nullptr;
            }
        }
    }
    // Connect to device and set up callbacks
    myClient = NimBLEDevice::createClient();
    myClient->setClientCallbacks(new MyClientCallback());
    myClient->setConnectTimeout(5);
    myClient->connect(myDevice);
    NimBLERemoteService* pService = myClient->getService("6a800001-b5a3-f393-e0a9-e50e24dcca9e");
    delay(10);
    NimBLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic("6a806050-b5a3-f393-e0a9-e50e24dcca9e");
    delay(10);

    // Set client callbacks
    if(!pCharacteristic->canNotify()){
      Serial.println("This characteristic does not support notifications");
    } else {
        Serial.println("This characteristic supports notifications");
        pCharacteristic->subscribe(true, onNotify);
        tft.setTextSize(2);
        tft.println("Connected!");
    }
}

// Loop function
void loop() {
    button1.loop();
    button2.loop();
}
