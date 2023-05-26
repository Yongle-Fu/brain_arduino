/*
 * 自定义通信协议
 * 支持arduino uno, stm32, esp32
 * @Url: [主控与编程板通信协议](https://brainco.yuque.com/qc91eq/lbx4ww/kngkgigdgbgghw3s)
 * @Author: Yongle Fu
 * @Date: 2023-05-12
 * version: 0.0.1
 */
#ifndef NM_Command_H_
#define NM_Command_H_

#include <Arduino.h>

enum class ResultCode : uint8_t {
  Success = 0,    // 指令成功执行
  NotFound = 1,   // 未找到指定设备/传感器
  Busy = 2,       // 该动作/指令正在执行中
  ParamError = 3, // 参数错误
  Timeout = 4,    // 动作超时或读取超时
  Unsupported = 5,// 未支持的指令
  Unknown = 6,    // 未知错误
  System = 7      // 系统故障
};

// 指令类型
enum class CommandType : uint8_t {
  Control = 0x01, // 控制指令，可控制手势动作，外置马达舵机等动作
  Write = 0x02,   // 向主机写入配置参数和其它内容等数据
  Read = 0x03,    // 从主控读取外接模块传感器数据，每次同步读取单个数据，详见指令说明
  Forward = 0x04, // 转发从其它接入编程板的主机数据内容，转发至主控板
  Extend = 0x05,  // 扩展指令
  Test = 0x06,    // 用于板级的硬件测试指令
  Event = 0x07,   // 用于事件传递
  Subscription = 0x08, // 订阅，订阅数据主动连续上报，预留
};

// 控制类型
enum class ControlType : uint8_t {
  Finger = 0x01,  // 手指动作控制
  Gesture = 0x02, // 手势动作控制
  Led = 0x03,     // LED控制
  Motor = 0x04,   // 马达控制
  GPIO = 0x05,    // GPIO控制
  Servo = 0x06,   // 舵机控制
  Car = 0x07,     // 小车控制
};

// Read对象类型
enum class ReadObjectType : uint8_t {
  GPIO = 0x01, // GPIO读取
  Sensor = 0x02, // 传感器读取
  AIO = 0x03, // 模拟值读取
  Finger = 0x04, // 手指位置读取
  IRKey = 0x05, // 红外按键读取
};

// Read参数类型
enum class ReadObjectParamType : uint8_t {
  Data = 0x00, // 读取传感器输出值
  Status = 0x01, // 读取传感器状态是否就绪，0:未就绪，1：就绪
};

// 传感器编号
enum class SensorType : uint8_t {
  SoftSmall = 1, // 小柔性传感器
  Hall = 2, // 霍尔传感器
  RGB = 3, // RGB传感器
  Infrared = 4, // 红外传感器
  Ultrasonic = 5, // 超声波传感器
  Temperature = 6, // 温度传感器
  Sound = 7, // 声音传感器
  EMG = 8, // 肌电信号传感器
  LED = 9, // LED控制
  Potentiometer = 10, // 旋转电位传感器
  Button = 11, // 按钮传感器
  SoftBig = 12, // 大柔性传感器
};

// 传感器参数类型
enum class SensorParamType : uint8_t {
  Level = 0x01, // 等级
  Distance = 0x02, // 距离
  Temperature = 0x03, // 温度
  RGB = 0x04, // RGB颜色值
  Voltage = 0x05, // 电压
  SwitchLevel = 0x06, // 开关等级
  Adc = 0x07, // ADC值
};

// 手指序号枚举
enum class FingerNumber : uint8_t {
  Thumb = 0x1,   // 大拇指
  Index = 0x2,   // 食指
  Middle = 0x3,  // 中指
  Ring = 0x4,    // 无名指
  Little = 0x5,  // 小指
};

// 手势序号枚举
enum class GestureNumber : uint8_t {
  Reset = 0,     // 重置手势
  Pinch = 1,     // 捏取
  Grasp = 2,     // 抓握手势
  SidePinch = 3, // 侧捏取
  Tripod = 4,    // 三脚架
  Wave = 5,      // 波浪
  Victory = 6,   // 胜利
  Ok = 7         // OK
};

// 马达编号枚举
enum class MotorNumber : uint8_t {
  Motor1 = 0,    // 马达1
  Motor2 = 1     // 马达2
};

enum class LedNumber : uint8_t {
  Led1 = 0,    // LED 1
  Led2 = 1     // LED 2
};

enum class ServoNumber : uint8_t {
  Servo1 = 0,    // 舵机1
  Servo2 = 1     // 舵机2
};

enum class CarAction : uint8_t {
  Forward = 1,    // 前进
  Backward = 2,   // 后退
  Left = 3,       // 左转
  Right = 4       // 右转
};

enum class GPIOType : uint8_t {
  Voltage = 0x00, // 输出电平
  PWM = 0x01,     // PWM输出
};

enum class GPIOMode : uint8_t {
  Output = 0x00, // 输出模式
  Input = 0x01,  // 输入模式
};

enum class GPIOLevel : uint8_t {
  Low = 0x00,  // 输出低电平
  High = 0x01, // 输出高电平
};

enum class IRKeyCode : uint8_t {
  None = 0,     // 无按键
  Key1 = 1,     // '1' 键
  Key2 = 2,     // '2' 键
  Key3 = 3,     // '3' 键
  Key4 = 4,     // '4' 键
  Key5 = 5,     // '5' 键
  Key6 = 6,     // '6' 键
  Key7 = 7,     // '7' 键
  Key8 = 8,     // '8' 键
  Key9 = 9,     // '9' 键
  Key0 = 10,    // '0' 键
  Power = 11,   // Power 键,
  Plus = 12,    // '+' 键
  Minus = 13,   // '-' 键
  A = 14,       // 'A' 键
  B = 15,       // 'B' 键
  C = 16,       // 'C' 键
  D = 17,       // 'D' 键
  E = 18,       // 'E' 键
  F = 19,       // 'F' 键
  Func1 = 20,   // FUNC1 键
  Func2 = 21,   // FUNC2 键
  Func3 = 22,   // FUNC3 键
};

enum class InterfaceCode : uint8_t {
  A = 0,     // 接口A
  B = 1,     // 接口B
  C = 2,     // 接口C
  D = 3,     // 接口D
  E = 4,     // 接口E
  F = 5      // 接口F, 高级接口
};

struct FingerControl {
  ControlType type = ControlType::Finger;
  uint8_t pos[5]; // 控制5个手指位置，动作位置百分比, 0~101, 0表示该手指无动作
};

struct GestureControl {
  ControlType type = ControlType::Gesture;
  GestureNumber no;                      // 手势编号
  uint8_t pos;                           // 动作位置百分比, 0~100
};

struct LedControl {
  ControlType type = ControlType::Led;
  LedNumber no = LedNumber::Led1;         // LED编号
  uint8_t rgb[3];                        
};

struct MotorControl {
  ControlType type = ControlType::Motor;
  MotorNumber no;
  uint8_t direction; // 方向：0-前进，1-后退
  uint8_t speed; // 转速度, [0-100]
  uint8_t angle; // 转角度, [0-100]
  uint8_t time;  // 时间, [0-255]
};

struct GPIOControl {
  ControlType type = ControlType::GPIO;  
  uint8_t no = 0;                        // GPIO 编号[1-15] 
  // GPIOType gpio_type;                 // GPIO 类型（电平输出或 PWM 输出）
  GPIOLevel level;                       // 输出值（电平输出为高电平、低电平）
};

struct ServoControl {
  ControlType type = ControlType::Servo;
  ServoNumber no;
  uint8_t angle; // 转角度, [0-100]
};

struct CarControl {
  ControlType type = ControlType::Car;
  CarAction no;
  uint8_t speed; // 速度, [0-100]
  uint8_t time;  // 时间, [0-255]
};

struct NMCommand {
  int msgId;
  int cmd;
  int resultCode;
  bool isBytes; // bytes or string
  bool isSync;
  uint8_t* params;
  int length;
};

void nm_setup();
bool nm_read_available();
bool nm_write_available();
void nm_serial_read();

// ******************************************Control Methods******************************************
void nm_set_finger(FingerControl* control);
void nm_set_gesture(GestureControl* control);
void nm_set_led(LedControl* control);
void nm_set_motor(MotorControl* control);
void nm_set_gpio(GPIOControl* control);
void nm_set_servo(ServoControl* control);
void nm_set_car(CarControl* control);

// *******************************************Read Methods********************************************
typedef void (*ValueArrayCallback)(const byte* buff, byte length); 
// typedef void (*ValueCB)(const byte* bytes, uint8_t len);

bool nm_is_sensor_ready(SensorType sensorType, InterfaceCode interface);
bool nm_is_sensor_on(SensorType sensorType, InterfaceCode interface);

uint8_t  nm_get_sensor_byte(SensorType sensorType, InterfaceCode interface);
uint8_t* nm_get_sensor_bytes(SensorType sensorType, InterfaceCode interface);

int16_t nm_get_sensor_int16(SensorType sensorType, InterfaceCode interface);

// RGB color
uint8_t nm_get_rgb_value(InterfaceCode interface, uint8_t index);
uint8_t* nm_get_rgb_values(InterfaceCode interface);

bool nm_get_gpio(uint8_t no);// Digital, [1-15], [0-1], LOW-HIGH
uint16_t nm_get_aio(uint8_t no); // Analog,  [1-10], [0-4095]

uint8_t nm_get_finger(FingerNumber no);
uint8_t nm_get_ir_key(); // return IR key code
bool nm_is_ir_key_pressed(uint8_t key);

// ******************************************Print Methods******************************************
String strCommandType(CommandType type);
String strControlType(ControlType type);
String strSensorType(SensorType type);
String strFingerNumber(FingerNumber no);
String strGestureNumber(GestureNumber no);
void printHexBytes(String prefix, const byte* buff, int len);

#endif