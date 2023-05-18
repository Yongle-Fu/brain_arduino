#include <Arduino.h>
#include "nm_command.h"

void setup() {
  nm_setup();
  // delay(1000);
  
  // 发送手指控制
  fingerAction(FingerNumber::Thumb, 50); // 0~100
  Serial.println("fingerAction done");
  // delay(10000);
  
  // 发送手势控制
  gestureAction(GestureNumber::Wave, 90); // 0~100
  Serial.println("gestureAction Wave done");
  // delay(10000);

  gestureAction(GestureNumber::Pinch, 80); // 0~100
  Serial.println("gestureAction Pinch done");
  // delay(10000);

  nm_get_rgb_values(0);
  Serial.println("nm_get_rgb_values done");

  nm_get_rgb_value(0, 0);
  Serial.println("nm_get_rgb_value done");

  nm_get_rgb_value(0, 1);
  Serial.println("nm_get_rgb_value done");

  nm_get_rgb_value(0, 2);
  Serial.println("nm_get_rgb_value done");

  nm_get_sensor_byte(SensorType::Button, 0);
  Serial.println("nm_get_sensor_byte Button done"); 

  nm_get_sensor_int16(SensorType::Temperature, 0); 
  Serial.println("nm_get_sensor_byte Temperature done"); 

  nm_get_sensor_int16(SensorType::Ultrasonic, 0); 
  Serial.println("nm_get_sensor_byte Ultrasonic done");
}

void loop() {
}

// 手指控制
// position: 0~100, 手指动作位置百分比
void fingerAction(FingerNumber fingerNum, uint8_t position) {
  nm_set_finger(static_cast<uint8_t>(fingerNum), position);
}

// 手势控制
// position: 0~100, 动作位置百分比
void gestureAction(GestureNumber gestureNum, uint8_t position) {
  nm_set_gesture(static_cast<uint8_t>(gestureNum), position);
}
