#include <Arduino.h>
#include "nm_command.h"

void setup() {
  nm_setup();
  delay(1000);
  
  // 发送手指控制
  fingerAction(FingerNumber::Thumb, 50); // 0~100


  while (nm_isReadAvailable())
  {
    asyncReadSensorValue(SensorType::Temperature);
  }
  
  // 发送手势控制
  // gestureAction(GestureNumber::Wave, 90); // 0~100
  // gestureAction(GestureNumber::Pinch, 80); // 0~100
}

void loop() {
  nm_loop();
  // asyncRead();
  // asyncReadSensorValue(SensorType::RGB);
  // asyncReadSensorValue(SensorType::Temperature);
  // asyncReadSensorValue(SensorType::Ultrasonic);
  // asyncReadSensorValue(SensorType::Button);
}

// Flag to track if the async operation is completed
static bool asyncReadCompleted = false;

void asyncRead() {
  if (asyncReadCompleted || !nm_isReadAvailable()) return;
  nm_readRGB(0, [](const byte* buff, byte length) {
    Serial.print("async read: ");
    for (int i = 0; i < length; i++) {
      Serial.print(buff[i], HEX);
      Serial.print(", ");
    }
    Serial.println("");
  });
  asyncReadCompleted = true;
}

void asyncReadSensorValue(SensorType senorType) {
  if (asyncReadCompleted || !nm_isReadAvailable()) return;
  // const uint8_t sensor_type = static_cast<uint8_t>(senorType);
  // nm_readSensorStatus(senorType, 0, [](const byte* buff, byte length) {
  nm_readSensorValue(senorType, 0, [](const byte* buff, byte length) {
    // Serial.print("async read " + strControlType(sensor_type) + ": ");
    Serial.print("async read: ");
    for (int i = 0; i < length; i++) {
      Serial.print(buff[i], HEX);
      Serial.print(", ");
    }
    Serial.println("");
  });
  asyncReadCompleted = true;
}

// 手指控制
// position: 0~100, 手指动作位置百分比
void fingerAction(FingerNumber fingerNum, uint8_t position) {
  nm_fingerAction(static_cast<uint8_t>(fingerNum), position);
  // void nm_readRGB(uint8_t interface, ValueCB cb)
}

// 手势控制
// position: 0~100, 动作位置百分比
void gestureAction(GestureNumber gestureNum, uint8_t position) {
  nm_gestureAction(static_cast<uint8_t>(gestureNum), position);
}
