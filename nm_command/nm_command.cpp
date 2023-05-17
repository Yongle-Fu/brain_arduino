#include <Arduino.h>
#include <SoftwareSerial.h>
#include "nm_command_parser.h"
#include "nm_command_writer.h"

static CommandParser parser;
static CommandWriter writer;

SoftwareSerial commSerial(12, 11); // RX, TX

// if use HardwareSerial
// #define commSerial Serial1  

void nm_setup() {
  Serial.begin(115200);
  commSerial.begin(115200);
  // Serial.begin(9600);
  // commSerial.begin(9600);

  // if use HardwareSerial
  // pinMode(PB2, OUTPUT);
  // pinMode(PB3, INPUT_PULLUP);

  Serial.println("---------setup---------");
  // delay(1000);

  //设置缓冲区大小, 预计指令的最大的长度
  parser.setBufferSize(30);  // 6 + data_length

  // Magic: 固定字节 0x5a，以此识别帧头
  // Version: 协议版本号 版本号由01开始递增
  // 设置开始指令开始的匹配
  byte start[2] = { 0x5a, 0x01 };
  parser.setStart(start, 2);
  writer.setStart(start, 2);

  //设置指令处理回调
  parser.setResolveCommandCallback([](const NMCommand& command) {
    Serial.print("Response: cmd=");
    Serial.print(strCommandType(command.cmd));
    Serial.print(", retCode=");
    Serial.print(command.resultCode);
    if (!command.isBytes || !command.isSync) {
      Serial.print(", isBytes=");
      Serial.print(command.isBytes);
      Serial.print(", isSync=");
      Serial.print(command.isSync);
    }

    if (command.length > 0) Serial.print(", Params=");
    for (int i = 0; i < command.length; i++) {
      Serial.print(command.params[i], HEX);
      Serial.print(" ");
    }
    Serial.println("\n");
    delay(50);
    writer.processMessageQueue();
  });

  //设置发送数据的回调实现
  writer.setWriteCallback([](byte* buff, byte length) {
    Serial.print("Write, ");
    for (int i = 0; i < length; i++) {
      Serial.print("0x");
      Serial.print(buff[i], HEX);
      if (i < length-1) Serial.print(", ");
    }
    //通过串口发送
    commSerial.write(buff, length);
    Serial.println("");
  });
}

void nm_loop() {
  while (commSerial.available()) {
    //接收指令, 直接跟串口结合
    byte b = commSerial.read();

    Serial.print("0x");
    Serial.print(b, HEX);
    if (commSerial.available()) Serial.print(", ");
    else Serial.println("");
    
    parser.onReceiveData(b);
  }
}

// 手指控制
// position: 0~100, 动作位置百分比
void nm_fingerAction(uint8_t finger, uint8_t position) {
  if (position > 100) position = 100;
  NMCommand command;
  command.cmd = static_cast<uint8_t>(CommandType::Control);
  byte* params = new byte[3]{ static_cast<uint8_t>(ControlType::Finger), finger, position };
  command.params = params;
  writer.addToMessageQueue(&command);
}

// 手势控制
// position: 0~100, 动作位置百分比
void nm_gestureAction(uint8_t gesture, uint8_t position) {
  if (position > 100) position = 100;
  NMCommand command;
  command.cmd = static_cast<uint8_t>(CommandType::Control);
  byte* params = new byte[3]{ static_cast<uint8_t>(ControlType::Gesture), gesture, position };
  command.params = params;
  writer.addToMessageQueue(&command);
}

// LED 控制
void nm_ledControl(LedNumber ledNum, int r, int g, int b) {
  // 根据 LED 编号和亮度值执行对应的操作
  switch (ledNum) {
    case LedNumber::Led1:
      // 控制 LED1
      // ...
      break;
    case LedNumber::Led2:
      // 控制 LED2
      // ...
      break;
    // 可以在此处添加更多的 LED 编号
    default:
      break;
  }
}

// 马达控制
void motorControl(byte* params, int length) {
  if (length < 2) {
    // 参数不足，无法执行操作
    return;
  }

  MotorNumber motorNum = static_cast<MotorNumber>(params[0]);
  uint8_t motorValue = params[1];

  // 根据马达编号和速度值执行对应的操作
  switch (motorNum) {
    case MotorNumber::Motor1:
      // 控制 Motor1
      // ...
      break;
    case MotorNumber::Motor2:
      // 控制 Motor2
      // ...
      break;
    // 可以在此处添加更多的马达编号
    default:
      break;
  }
}

void gpioControl() {
  // 定义一个 GPIOControl 结构体对象
  GPIOControl gpioCtrl;

  // 设置 GPIOControl 结构体对象的成员值
  gpioCtrl.type = ControlType::GPIO;
  gpioCtrl.object_num = 1;
  gpioCtrl.gpio_type = GPIOType::Voltage;
  gpioCtrl.gpio_mode = GPIOMode::Output;
  gpioCtrl.value = static_cast<uint8_t>(GPIOValue::High);

  // 根据结构体对象的成员值进行 GPIO 输出
  if (gpioCtrl.gpio_mode == GPIOMode::Output) {
    switch (gpioCtrl.gpio_type) {
      case GPIOType::Voltage:
        if (gpioCtrl.value == static_cast<uint8_t>(GPIOValue::High)) {
          digitalWrite(gpioCtrl.object_num, HIGH);
        } else {
          digitalWrite(gpioCtrl.object_num, LOW);
        }
        break;
      case GPIOType::PWM:
        analogWrite(gpioCtrl.object_num, map(gpioCtrl.value, 0, 100, 0, 255));
        break;
    }
  }
}

// 枚举类型转换为字符串
const char* strCommandType(uint8_t value) {
  CommandType type = static_cast<CommandType>(value);
  switch (type) {
    case CommandType::Control:
      return "Control";
    case CommandType::Write:
      return "Write";
    case CommandType::Read:
      return "Read";
    case CommandType::Forward:
      return "Forward";
    case CommandType::Extend:
      return "Extend";
    case CommandType::Test:
      return "Test";
    case CommandType::Event:
      return "Event";
    case CommandType::Subscription:
      return "Subscription";
    default:
      return "Unknown";
  }
}

// 枚举类型转换为字符串
const char* strControlType(uint8_t value) {
  ControlType type = static_cast<ControlType>(value);
  switch (type) {
    case ControlType::Finger:
      return "Finger";
    case ControlType::Gesture:
      return "Gesture";
    case ControlType::Led:
      return "Led";
    case ControlType::Motor:
      return "Motor";
    case ControlType::GPIO:
      return "GPIO";
    case ControlType::Servo:
      return "Servo";
    default:
      return "Unknown";
  }
}

// 枚举类型转换为字符串
const char* strFingerNumber(uint8_t value) {
  FingerNumber type = static_cast<FingerNumber>(value);
  switch (type) {
    case FingerNumber::Thumb:
      return "Thumb";
    case FingerNumber::Index:
      return "Index";
    case FingerNumber::Middle:
      return "Middle";
    case FingerNumber::Ring:
      return "Ring";
    case FingerNumber::Little:
      return "Little";
    default:
      return "Unknown";
  }
}

// 枚举类型转换为字符串
const char* strGestureNumber(uint8_t value) {
  GestureNumber type = static_cast<GestureNumber>(value);
  switch (type) {
    case GestureNumber::Reset:
      return "Reset";
    case GestureNumber::Pinch:
      return "Pinch";
    case GestureNumber::Grasp:
      return "Grasp";
    case GestureNumber::SidePinch:
      return "SidePinch";
    case GestureNumber::Tripod:
      return "Tripod";
    case GestureNumber::Wave:
      return "Wave";
    case GestureNumber::Victory:
      return "Victory";
    case GestureNumber::Ok:
      return "Ok";
    default:
      return "Unknown";
  }
}

/*
// 枚举类型转换为字符串
const char* enum(GPIOType value) {
  switch (value) {
    case GPIOType::Voltage:
      return "Voltage";
    case GPIOType::PWM:
      return "PWM";
    default:
      return "Unknown";
  }
}

// 枚举类型转换为字符串
const char* enum(GPIOMode value) {
  switch (value) {
    case GPIOMode::Output:
      return "Output";
    case GPIOMode::Input:
      return "Input";
    default:
      return "Unknown";
  }
}

// 枚举类型转换为字符串
const char* enum(GPIOValue value) {
  switch (value) {
    case GPIOValue::Low:
      return "Low";
    case GPIOValue::High:
      return "High";
    default:
      return "Unknown";
  }
}

// 枚举类型转换为字符串
const char* enum(MotorNumber value) {
  switch (value) {
    case MotorNumber::Motor1:
      return "Motor1";
    case MotorNumber::Motor2:
      return "Motor2";
    default:
      return "Unknown";
  }
}
*/