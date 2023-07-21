#ifndef NM_M_BLOCK_H_
#define NM_M_BLOCK_H_

#include "nm_command.h"   
#include "logger.h"

void setFinger(FingerNumber fingerNum, uint8_t position);
void setAllFinger(uint8_t pos1, uint8_t pos2, uint8_t pos3, uint8_t pos4, uint8_t pos5);
void setGesture(GestureNumber gestureNum, uint8_t position);
void setLed(LedNumber ledNumber, uint8_t r, uint8_t g, uint8_t b);
void setMotor(MotorNumber motorNumber, uint8_t direction, uint8_t speed, uint8_t angle, uint8_t time);
void setGPIO(uint8_t gpioNumber, GPIOLevel level);
void setServo(ServoNumber servoNumber, uint8_t angle);
void setCar(CarAction carAction, uint8_t speed, uint8_t time);

bool isRedColor(int red, int green, int blue);
bool isGreenColor(int red, int green, int blue);
bool isBlueColor(int red, int green, int blue);
bool isInColorRange(InterfaceCode port, int index);

uint8_t getSoftBig(int no);
void setLedByIndex(uint8_t index);

#endif
