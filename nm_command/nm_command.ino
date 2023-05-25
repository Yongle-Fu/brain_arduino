#include <Arduino.h>
#include "nm_command.h"

void setup() {
  nm_setup();
  
  // 发送手指
  setFinger(FingerNumber::Thumb, 50); // 0~100
  Serial.println("fingerAction done");
  
  // 发送手势控制
  setGesture(GestureNumber::Wave, 90); // 0~100
  Serial.println("gestureAction Wave done");

  setGesture(GestureNumber::Pinch, 80); // 0~100
  Serial.println("gestureAction Pinch done");

  setLed(LedNumber::Led1, 100, 100, 100); 
  setMotor(MotorNumber::Motor1, 0, 80, 90, 200);
  setGPIO(1, GPIOLevel::High);
  setServo(ServoNumber::Servo1, 45);
  setCar(CarAction::Forward, 50, 1000);

  nm_is_sensor_ready(SensorType::RGB, InterfaceCode::A);

  nm_is_sensor_on(SensorType::Hall, InterfaceCode::A);
  nm_is_sensor_on(SensorType::Infrared, InterfaceCode::A);
  nm_is_sensor_on(SensorType::Sound, InterfaceCode::A);
  nm_is_sensor_on(SensorType::Button, InterfaceCode::A);

  nm_get_gpio(10);
  nm_get_aio(10);
  nm_get_finger(FingerNumber::Ring);
  nm_get_ir_key();

  nm_get_rgb_values(InterfaceCode::A);
  Serial.println("nm_get_rgb_values done");

  nm_get_rgb_value(InterfaceCode::A, 0);
  Serial.println("nm_get_rgb_value done");

  // nm_get_rgb_value(InterfaceCode::A, 1);
  // Serial.println("nm_get_rgb_value done");

  // nm_get_rgb_value(InterfaceCode::A, 2);
  // Serial.println("nm_get_rgb_value done");

  nm_get_sensor_byte(SensorType::Button, InterfaceCode::A);
  Serial.println("nm_get_sensor_byte Button done"); 
  
  nm_get_sensor_int16(SensorType::Temperature, InterfaceCode::A); 

  Serial.println("nm_get_sensor_byte Temperature done"); 
  nm_get_sensor_int16(SensorType::Ultrasonic, InterfaceCode::A); 

  Serial.println("setup done");
}

void loop() {}

// 手指
void setFinger(FingerNumber fingerNum, uint8_t position) {
  if (position > 100) position = 100;
  FingerControl fingerControl;
  auto index = static_cast<uint8_t>(fingerNum) - 1;
  for (int i = 0; i < 5; i++) {
    fingerControl.pos[i] = i == index ? position:  0;
  }

  nm_set_finger(&fingerControl);
}

// 手势控制
void setGesture(GestureNumber gestureNum, uint8_t position) {
  GestureControl control;
  control.no = gestureNum; // 手指编号
  control.pos = position;

  nm_set_gesture(&control);
}

// 封装 nm_set_led 函数调用代码
void setLed(LedNumber ledNumber, uint8_t r, uint8_t g, uint8_t b) {
  LedControl ledControl;
  ledControl.no = ledNumber;
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
void setServo(ServoNumber servoNumber, uint8_t angle) {
  ServoControl servoControl;
  servoControl.no = servoNumber;
  servoControl.angle = angle;

  nm_set_servo(&servoControl);
}

// 封装 nm_set_car 函数调用代码
void setCar(CarAction carAction, uint8_t speed, uint8_t time) {
  CarControl carControl;
  carControl.no = carAction;
  carControl.speed = speed;
  carControl.time = time;

  nm_set_car(&carControl);
}