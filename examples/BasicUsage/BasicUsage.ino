#include <Arduino.h>
#include "NeuroMaster.h"

void setup() {
  programSetup();
  testLED();
  // testGestures();
  // testReadAio();
}

void loop() {}
void _loop() {}

static void testLED(void) {
    for(;;) {
      setGesture(static_cast<GestureNumber>(6), 100);
      Serial.println("gesture move");
      _delay(2);
      setGesture(static_cast<GestureNumber>(7), 100);
      _delay(2);
      setGesture(GestureNumber::Reset, 0);
      _delay(1);
    }
}

static void testGestures(void) {
  // ******************************************Control Methods******************************************
  // 控制手指
  setFinger(FingerNumber::Thumb, 50); // 0~100
  Logger::print(INFO, "fingerAction done");

  setAllFinger(10, 20, 30, 40, 50); // 0~100
  
  // 控制手势
  setGesture(GestureNumber::Wave, 90); // 0~100
  Logger::print(INFO, "gestureAction Wave done");

  setGesture(GestureNumber::Pinch, 80); // 0~100
  Logger::print(INFO, "gestureAction Pinch done");

  setLed(100, 100, 100, InterfaceCode::A); 
  setMotor(MotorNumber::Motor1, 0, 80, 90, 200);

  setGPIO(1, GPIOLevel::High);
  setServo(ServoNumber::Servo1, 45);
  Logger::print(INFO, "setup done 0");
  return;

  setCar(CarAction::Forward, 50, 100); // direction, spped, time

  // *******************************************Read Methods********************************************
  isSensorReady(SensorType::RGB, InterfaceCode::A);

  isSensorOn(SensorType::Hall, InterfaceCode::A);
  isSensorOn(SensorType::Infrared, InterfaceCode::A);
  isSensorOn(SensorType::Button, InterfaceCode::A);

  isGpioHigh(10);
  getAioValue(10);
  getFingerValue(FingerNumber::Ring);
  getIrKey();

  getRgbValues(InterfaceCode::A);
  Logger::print(INFO, "getRgbValues done");

  isInColorRange(InterfaceCode::A, 0);

  getSensorByte(SensorType::Sound, InterfaceCode::A);
  Logger::print(INFO, "getSensor Sound done"); 
  
  getSensorInt16(SensorType::Temperature, InterfaceCode::A); 
  Logger::print(INFO, "getSensor Temperature done"); 
  
  getSensorInt16(SensorType::Ultrasonic, InterfaceCode::A); 
  Logger::print(INFO, "getSensor Ultrasonic done");

  getSoftBig(1);

  Logger::print(INFO, "setup done");
}

static void testReadAio() {
  for(int count=0;count<30;count++){
      Serial.println("A1");
      Serial.println(getAioValue(1));
      Serial.println("A2");
      Serial.println(getAioValue(2));
      Serial.println("A3");
      Serial.println(getAioValue(3));
      Serial.println("A4");
      Serial.println(getAioValue(4));
      Serial.println("A5");
      Serial.println(getAioValue(5));
      _delay(2);
  }
}

void _delay(float seconds) {
  unsigned long endTime = millis() + seconds * 1000;
  while(millis() < endTime) _loop();
}