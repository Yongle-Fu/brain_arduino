#include <Arduino.h>
#include "nm_command.h"

void setup() {
  nm_setup();
  
  delay(1000);
  
  // 发送手指控制
  fingerAction(FingerNumber::Thumb, 50); // 0~100
  // delay(3000);

  // 发送手势控制
  gestureAction(GestureNumber::Wave, 90); // 0~100
  // delay(3000);

  gestureAction(GestureNumber::Pinch, 80); // 0~100
}

void loop() {
  nm_loop();
}

// 手指控制
// position: 0~100, 手指动作位置百分比
void fingerAction(FingerNumber fingerNum, uint8_t position) {
  nm_fingerAction(static_cast<uint8_t>(fingerNum), position);
}

// 手势控制
// position: 0~100, 动作位置百分比
void gestureAction(GestureNumber gestureNum, uint8_t position) {
  nm_gestureAction(static_cast<uint8_t>(gestureNum), position);
}
