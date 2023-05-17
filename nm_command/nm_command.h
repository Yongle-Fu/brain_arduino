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
  Servo = 0x06,   // 伺服电机控制
};

// read对象类型
enum class ReadObjectType : uint8_t {
  GPIO = 0x01, // GPIO读取
  Sensor = 0x02, // 传感器读取
  Interface = 0x03, // 接口读取
  AIO = 0x04, // 
};

// read参数类型
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
  Thumb = 1,   // 大拇指
  Index = 2,   // 食指
  Middle = 3,  // 中指
  Ring = 4,    // 无名指
  Little = 5,  // 小指
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

enum class GPIOType : uint8_t {
  Voltage = 0x00, // 输出电平
  PWM = 0x01,     // PWM输出
};

enum class GPIOMode : uint8_t {
  Output = 0x00, // 输出模式
  Input = 0x01,  // 输入模式
};

enum class GPIOValue : uint8_t {
  Low = 0x00,  // 输出低电平
  High = 0x01, // 输出高电平
};

const char* strCommandType(uint8_t value);
const char* strControlType(uint8_t value);
const char* strFingerNumber(uint8_t value);
const char* strGestureNumber(uint8_t value);
const char* strSensorType(uint8_t value);

struct GPIOControl {
  ControlType type = ControlType::GPIO;  // 控制类型
  uint8_t object_num = 0;                // GPIO 对象序号
  GPIOType gpio_type;                    // GPIO 类型（电平输出或 PWM 输出）
  GPIOMode gpio_mode = GPIOMode::Output; // GPIO 模式（输入或输出）
  uint8_t value = 0;                     // 输出值（电平输出为 0 或 1，PWM 输出为 0-100 的占空比）
};

struct FingerData {
  byte fingerNum; // 手指编号
  byte fingerPos; // 手指动作位置百分比
};

struct GestureData {
  byte gestureNum; // 手势编号
  byte gesturePos; // 手势动作位置百分比
};

struct LedData {
  byte ledNum; // LED编号
  byte red;    // 颜色R值
  byte green;  // 颜色G值
  byte blue;   // 颜色B值
};

struct MotorData {
  byte motorNum;   // 马达编号
  byte direction;  // 转动方向
  byte speed;      // 转速度
};

struct GpioData {
  byte gpioNum;       // GPIO编号
  byte outputType;    // GPIO输出类型
  byte outputValue;   // 输出值（详见GPIO输出说明）
};

struct ServoData {
  byte servoNum; // servo编号
  byte angle;    // 转动角度
};

struct Command {
  CommandType type; // 控制类型
  byte objNum;       // 对象序号
  union {
    FingerData finger; // 手指控制数据
    GestureData gesture; // 手势控制数据
    LedData led; // LED控制数据
    MotorData motor; // 马达控制数据
    GpioData gpio; // GPIO控制数据
    ServoData servo; // 伺服电机控制数据
  } data; // 参数数据
};

struct NMCommand {
  int msgId;
  int cmd;
  int resultCode;
  bool isBytes; // bytes or string
  bool isSync;
  byte* params;
  int length;
};

void nm_setup();
void nm_loop();

// ******************************************Control*********************************************
// 手指控制, position: 0~100, 动作位置百分比
void nm_fingerAction(uint8_t finger, uint8_t position);
// 手势控制, position: 0~100, 动作位置百分比
void nm_gestureAction(uint8_t gesture, uint8_t position);


// ******************************************Read Value******************************************
// 检查是否可以发送数据
bool nm_isReadAvailable();

// Hall = 2, // 霍尔传感器
// Infrared = 4, // 红外传感器
// Sound = 7, // 声音传感器
// Button = 11, // 按钮传感器
bool nm_isSensorOn(SensorType sensorType); // 1 bytes, OFF-ON

uint16_t nm_readUInt16(SensorType sensorType); 

// 温度	6	Temperature	<int16>
// [-200 - 1000](0.1℃)	1
int16_t nm_readInt16(SensorType sensorType); 

// 超声波	5	Distance	<uint16>
// [0-1000]
// (0.1cm)	1

typedef void (*ValueCB)(const byte* bytes, uint8_t len);
// interface: A-F, 0-6
void nm_readRGB(uint8_t interface, ValueCB cb); // 3 bytes, R,G,B <uint8>

// interface: A-F, 0-6
void nm_readSensorValue(SensorType sensorType, uint8_t interface, ValueCB cb);
void nm_readSensorStatus(SensorType sensorType, uint8_t interface, ValueCB cb);

#endif