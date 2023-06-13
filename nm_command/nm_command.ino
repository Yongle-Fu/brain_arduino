#include <Arduino.h>
#include "nm_mblock.h"

void _delay(float seconds) {
  long endTime = millis() + seconds * 1000;
  while(millis() < endTime) _loop();
}

void setup() {
  nm_setup();
  
  // ******************************************Control Methods******************************************
  // 控制手指
  setFinger(FingerNumber::Thumb, 50); // 0~100
  Serial.println("fingerAction done");

  setAllFinger(10, 20, 30, 40, 50); // 0~100
  
  // 控制手势
  setGesture(GestureNumber::Wave, 90); // 0~100
  Serial.println("gestureAction Wave done");

  setGesture(GestureNumber::Pinch, 80); // 0~100
  Serial.println("gestureAction Pinch done");

  setLed(LedNumber::Led1, 100, 100, 100); 
  setMotor(MotorNumber::Motor1, 0, 80, 90, 200);

  setGPIO(1, GPIOLevel::High);
  setServo(ServoNumber::Servo1, 45);
  Serial.println("setup done 0");
  return;

  setCar(CarAction::Forward, 50, 1000);
  // setCar(static_cast<CarAction>(/*{direction}*/), /*{speed}*/, 0);
  // setCar(static_cast<CarAction>(/*{direction}*/), /*{speed}*/, /*{time}*/);

  // *******************************************Read Methods********************************************
  nm_is_sensor_ready(SensorType::RGB, InterfaceCode::A);

  nm_is_sensor_on(SensorType::Hall, InterfaceCode::A);
  nm_is_sensor_on(SensorType::Infrared, InterfaceCode::A);
  nm_is_sensor_on(SensorType::Button, InterfaceCode::A);

  nm_get_gpio(10);
  nm_get_aio(10);
  nm_get_finger(FingerNumber::Ring);
  nm_get_ir_key();

  nm_get_rgb_values(InterfaceCode::A);
  Serial.println("nm_get_rgb_values done");

  isInColorRange(InterfaceCode::A, 0);

  nm_get_sensor_byte(SensorType::Sound, InterfaceCode::A);
  Serial.println("nm_get_sensor_byte Sound done"); 
  
  nm_get_sensor_int16(SensorType::Temperature, InterfaceCode::A); 
  Serial.println("nm_get_sensor_byte Temperature done"); 
  
  nm_get_sensor_int16(SensorType::Ultrasonic, InterfaceCode::A); 
  Serial.println("nm_get_sensor_byte Ultrasonic done"); 

  getSoftBig(1);

  Serial.println("setup done");
}

void loop() {}
void _loop() {}