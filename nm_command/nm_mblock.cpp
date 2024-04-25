#include "nm_mblock.h"   

// 手指
void setFinger(FingerNumber fingerNum, uint8_t position) {
  if (position > 100) position = 100;

  FingerControl fingerControl;
  auto index = static_cast<uint8_t>(fingerNum) - 1;
  for (int i = 0; i < 5; i++) {
    fingerControl.pos[i] = i == index ? position+1: 0;
  }

  nm_set_finger(&fingerControl);
}

void setAllFinger(uint8_t pos1, uint8_t pos2, uint8_t pos3, uint8_t pos4, uint8_t pos5) {
  if (pos1 > 100) pos1 = 100;
  if (pos2 > 100) pos2 = 100;
  if (pos3 > 100) pos3 = 100;
  if (pos4 > 100) pos4 = 100;
  if (pos5 > 100) pos5 = 100;

  FingerControl fingerControl;
  fingerControl.pos[0] = pos1 + 1;
  fingerControl.pos[1] = pos2 + 1;
  fingerControl.pos[2] = pos3 + 1;
  fingerControl.pos[3] = pos4 + 1;
  fingerControl.pos[4] = pos5 + 1;

  nm_set_finger(&fingerControl);
}

// 手势控制
void setGesture(GestureNumber gestureNum, uint8_t position) {
  GestureControl control;
  control.no = gestureNum;
  control.pos = position;

  nm_set_gesture(&control);
}

// 封装 nm_set_led 函数调用代码
void setLed(uint8_t r, uint8_t g, uint8_t b, InterfaceCode port) {
  LedControl ledControl;
  ledControl.port = port;
  ledControl.rgb[0] = r;
  ledControl.rgb[1] = g;
  ledControl.rgb[2] = b;

  nm_set_led(&ledControl);
}

// 封装 nm_set_motor 函数调用代码
void setMotor(MotorNumber motorNumber, uint8_t direction, uint8_t speed, uint8_t angle, uint8_t time) {
  MotorControl motorControl;
  motorControl.no = motorNumber;
  motorControl.direction = direction;
  motorControl.speed = speed;
  motorControl.angle = angle;
  motorControl.time = time;

  nm_set_motor(&motorControl);
}

// 封装 nm_set_gpio 函数调用代码
void setGPIO(uint8_t gpioNumber, GPIOLevel level) {
  GPIOControl gpioControl;
  gpioControl.no = gpioNumber;
  gpioControl.level = level;

  nm_set_gpio(&gpioControl);
}

// 封装 nm_set_servo 函数调用代码
// 因为固件上未实现单独控制5个手势舵机的功能，这里需要借用单独控制手指动作的指令来实现
// 注意单独控制舵机的方向可能与运动方向不一致，需要进行判断及转换
void setServo(ServoNumber servoNumber, uint8_t angle) {
  ServoControl servoControl;
  if (servoNumber == static_cast<ServoNumber>(5)) {
      servoControl.no = static_cast<ServoNumber>(0);
      servoControl.angle = min(angle, 180);
      nm_set_servo(&servoControl);
  } else {
    uint8_t revert_table[5] = {1,0,1,1,0};
    uint8_t pos = 0, real_angle = 0;
    uint8_t index = (uint8_t)servoNumber;
    FingerControl fingerControl ;
  
    if (revert_table[index]) {
      real_angle = 180 - min(angle,180);
    } else {
      real_angle = min(angle, 180);
    }
    pos = real_angle * 100 / 180;
    for (int i = 0; i < 5; i++) {
      fingerControl.pos[i] = i == index ? pos+1: 0;
    }
    nm_set_finger(&fingerControl);
  }

}

// 封装 nm_set_car 函数调用代码
void setCar(CarAction carAction, uint8_t speed, uint8_t time) {
  CarControl carControl;
  carControl.no = carAction;
  carControl.speed = speed;
  carControl.time = time;

  nm_set_car(&carControl);
}

bool isRedColor(int red, int green, int blue) {
  int sum = red + green + blue;
  // 判断是否属于红色范围
  float ratio = (sum > 0) ? (float)red / sum : 0.0f;
  if (ratio > 0.5f) {
      return true; // 是红色
  } else {
      return false; // 不是红色
  }
}

bool isGreenColor(int red, int green, int blue) {
  int sum = red + green + blue;
  // 判断是否属于绿色范围
  float ratio = (sum > 0) ? (float)green / sum : 0.0f;
  if (ratio > 0.43f) {
      return true; // 是绿色
  } else {
      return false; // 不是绿色
  }
}

bool isBlueColor(int red, int green, int blue) {
  int sum = red + green + blue;
  // 判断是否属于蓝色范围
  float ratio = (sum > 0) ? (float)blue / sum : 0.0f;
  if (ratio > 0.49f) {
      return true; // 是蓝色
  } else {
      return false; // 不是蓝色
  }
}

bool isInColorRange(InterfaceCode port, int index) {
  byte* rgb = nm_get_rgb_values(port);
  if (index == 0) return isRedColor(rgb[0], rgb[1], rgb[2]);
  else if (index == 1) return isGreenColor(rgb[0], rgb[1], rgb[2]);
  else if (index == 2) return isBlueColor(rgb[0], rgb[1], rgb[2]);
  else return false;
}

uint8_t getSoftBig(int no) {
  if (no < 1 || no > 5) return 0;
  byte* values = nm_get_sensor_bytes(SensorType::SoftBig, InterfaceCode::F);
  return values[no-1];
}

void setLedByIndex(uint8_t index, InterfaceCode port) {
  const uint8_t colors[][3] = {
      {0, 0, 0},        // 熄灭
      {255, 0, 0},      // 红色
      {0, 255, 0},      // 绿色
      {0, 0, 255},      // 蓝色
      {255, 255, 255},  // 白色
      {255, 255, 0},    // 黄色
      {255, 192, 203},  // 粉色
      {128, 0, 128}     // 紫色
  };

  if (index < sizeof(colors) / sizeof(colors[0])) {
      setLed(colors[index][0], colors[index][1], colors[index][2], port);
  } else {
      Serial.println("Invalid index!");
  }
}
  
