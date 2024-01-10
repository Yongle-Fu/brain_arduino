#include <Arduino.h>
#include "nm_mblock.h"

void _delay(float seconds) {
  long endTime = millis() + seconds * 1000;
  while(millis() < endTime) _loop();
}

void tes_move_gesture(void)
{
  
  // ******************************************Control Methods******************************************
  // 控制手指
  setFinger(FingerNumber::Thumb, 50); // 0~100
  Logger::print_log(INFO, "fingerAction done");

  setAllFinger(10, 20, 30, 40, 50); // 0~100
  
  // 控制手势
  setGesture(GestureNumber::Wave, 90); // 0~100
  Logger::print_log(INFO, "gestureAction Wave done");

  setGesture(GestureNumber::Pinch, 80); // 0~100
  Logger::print_log(INFO, "gestureAction Pinch done");

  setLed(100, 100, 100, InterfaceCode::A); 
  setMotor(MotorNumber::Motor1, 0, 80, 90, 200);

  setGPIO(1, GPIOLevel::High);
  setServo(ServoNumber::Servo1, 45);
  Logger::print_log(INFO, "setup done 0");
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
  Logger::print_log(INFO, "nm_get_rgb_values done");

  isInColorRange(InterfaceCode::A, 0);

  nm_get_sensor_byte(SensorType::Sound, InterfaceCode::A);
  Logger::print_log(INFO, "nm_get_sensor_byte Sound done"); 
  
  nm_get_sensor_int16(SensorType::Temperature, InterfaceCode::A); 
  Logger::print_log(INFO, "nm_get_sensor_byte Temperature done"); 
  
  nm_get_sensor_int16(SensorType::Ultrasonic, InterfaceCode::A); 
  Logger::print_log(INFO, "nm_get_sensor_byte Ultrasonic done"); 

  getSoftBig(1);

  Logger::print_log(INFO, "setup done");

}
static void test_led_set(void)
{
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
static void test_read_aio(void)
{
  for(int count=0;count<30;count++){
      Serial.println("A1");
      Serial.println(nm_get_aio(1));
      Serial.println("A2");
      Serial.println(nm_get_aio(2));
      Serial.println("A3");
      Serial.println(nm_get_aio(3));
      Serial.println("A4");
      Serial.println(nm_get_aio(4));
      Serial.println("A5");
      Serial.println(nm_get_aio(5));
      _delay(2);
  }
}

void setup() {

  nm_setup();
  // tes_move_gesture();
  test_led_set();
  // test_read_aio();
}

void loop() {}
void _loop() {}