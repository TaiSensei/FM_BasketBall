/**
 * Most of this code was taken from https://github.com/MonomoriumP/Buttplug.io--Lelo
 * I have tested this with xtoys.app, it should work with buttplug.io but I haven't tested
 * For now it emulates a lovense edge and gives you two channels with 20 steps.
 * You should be able to use https://stpihkal.docs.buttplug.io/docs/stpihkal/protocols/lovense/ to emulate a different toy from lovense.
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Define the states
#define STATE_DEFAULT 0
#define STATE_TOUCH1 1
#define STATE_TOUCH2 2
#define STATE_TOUCH3 3
#define STATE_TOUCH4 4
#define STATE_TOUCH5 5
#define STATE_TOUCH1_TOUCH2 6
//          ================= Pins ================
//uint8_t mtr1 = 13;
//uint8_t mtr2 = 14;
//uint8_t batPin = 32;
uint8_t inputPin1 = 15;
uint8_t inputPin2 = 13;
uint8_t inputPin3 = 32;
uint8_t inputPin4 = 6;
uint8_t inputPin5 = 7;

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
BLECharacteristic* pRxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
static uint8_t messageBuf[64];

int vibration;
int vibration1;
int vibration2;

int state = STATE_DEFAULT; // Initialize the state to default
String output = "00"; // Initialize the output string to "00"

#define SERVICE_UUID           "50300001-0023-4bd4-bbd5-a6920e4c5653"
#define CHARACTERISTIC_RX_UUID "50300002-0023-4bd4-bbd5-a6920e4c5653"
#define CHARACTERISTIC_TX_UUID "50300003-0023-4bd4-bbd5-a6920e4c5653"


String getOutput()
{
  return output;
}

class MySerialCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      static uint8_t messageBuf[64];
      assert(pCharacteristic == pRxCharacteristic);
      std::string rxValue = pRxCharacteristic->getValue();

      // ... (existing code for handling received data)
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Global variables to keep track of button presses
bool touch1Pressed = false;
bool touch2Pressed = false;
bool touch3Pressed = false;
bool touch4Pressed = false;
bool touch5Pressed = false;

void updateState() {
    if (touch1Pressed && touch2Pressed) {
        state = STATE_TOUCH1_TOUCH2;
    } else if (touch1Pressed) {
        state = STATE_TOUCH1;
    } else if (touch2Pressed) {
        state = STATE_TOUCH2;
    } else if (touch3Pressed) {
        state = STATE_TOUCH3;
    } else if (touch4Pressed) {
        state = STATE_TOUCH4;
    } else if (touch5Pressed) {
        state = STATE_TOUCH5;
    } else {
        state = STATE_DEFAULT;
    }

    // Reset the button press variables
    touch1Pressed = false;
    touch2Pressed = false;
    touch3Pressed = false;
    touch4Pressed = false;
    touch5Pressed = false;
}

void updateOutput() {
    switch (state) {
        case STATE_TOUCH1:
            output = "01";
            break;
        case STATE_TOUCH2:
            output = "02";
            break;
        case STATE_TOUCH3:
            output = "03";
            break;
        case STATE_TOUCH4:
            output = "04";
            break;
        case STATE_TOUCH5:
            output = "05";
            break;
        case STATE_TOUCH1_TOUCH2:
            output = "11";
            break;
        case STATE_DEFAULT:
        default:
            output = "00";
            break;
    }
}


void gotTouch1() {
    touch1Pressed = true;
    updateState();
    updateOutput();
}

void gotTouch2() {
    touch2Pressed = true;
    updateState();
    updateOutput();
}

void gotTouch3() {
    touch3Pressed = true;
    updateState();
    updateOutput();
}

void gotTouch4() {
    touch4Pressed = true;
    updateState();
    updateOutput();
}

void gotTouch5() {
    touch5Pressed = true;
    updateState();
    updateOutput();
}

void setup() {
  pinMode(inputPin1, INPUT_PULLUP);
  pinMode(inputPin2, INPUT_PULLUP);
  pinMode(inputPin3, INPUT_PULLUP);
  pinMode(inputPin4, INPUT_PULLUP);
  pinMode(inputPin5, INPUT_PULLUP);

  // Create the BLE Device
  BLEDevice::init("LVS-EDGE"); // CONFIGURATION: The name has to match the actual device for xtoys to see it.

  // ... (existing code for BLE setup)
}

void loop() {
    // Check input states
    if (digitalRead(inputPin1) == LOW) {
        gotTouch1();
    }
    if (digitalRead(inputPin2) == LOW) {
        gotTouch2();
    }
    if (digitalRead(inputPin3) == LOW) {
        gotTouch3();
    }
    if (digitalRead(inputPin4) == LOW) {
        gotTouch4();
    }
    if (digitalRead(inputPin5) == LOW) {
        gotTouch5();
    }

    static unsigned long lastBatteryUpdate = 0;
    unsigned long currentTime = millis();

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(100); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
 
        oldDeviceConnected = deviceConnected;
    }


    if (currentTime - lastBatteryUpdate >= 100) { // Broadcast battery level every 1 second
        lastBatteryUpdate = currentTime;

        String StateValue = getOutput();
        memmove(messageBuf, StateValue.c_str(), 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
        delay(50);
        updateState();
        updateOutput();
    }
 
}
