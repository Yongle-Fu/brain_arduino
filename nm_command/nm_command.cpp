#include <Arduino.h>
#include <SoftwareSerial.h>
// #include <AltSoftSerial.h>
#include "nm_command_parser.h"
#include "nm_command_writer.h"

static CommandParser parser;
static CommandWriter writer;

#define MAX_READ_BUFF_SIZE 4
static int readBuffLen = 0;
static byte readBuff[MAX_READ_BUFF_SIZE];
static ValueArrayCallback readValueCb = NULL;

// #define hardwareSerialEnabled 1 
#ifdef hardwareSerialEnabled
  #define commSerial Serial1  
#else
  // AltSoftSerial commSerial(12, 11); // RX, TX
  SoftwareSerial commSerial(12, 11); // RX, TX
#endif

void nm_setup() {
  Serial.begin(115200);
  while (!Serial) ; // wait for serial monitor

  commSerial.begin(115200);
  // Serial.begin(9600);
  // commSerial.begin(9600);

#ifdef hardwareSerialEnabled
  pinMode(PB2, OUTPUT);
  pinMode(PB3, INPUT_PULLUP);
#endif  

  Serial.println("\n---------setup---------");
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
  parser.setMessageCallback([](NMCommand& command) {
    command.msgId = writer.sendingMsgId;
    Serial.print("msgId=" + String(command.msgId) + ", cmd=" + strCommandType(command.cmd));
    Serial.print(", retCode=" + String(command.resultCode));
    if (!command.isBytes || !command.isSync) {
      Serial.print(", isBytes=" + String(command.isBytes));
      Serial.print(", isSync=" + String(command.isSync));
    }
    if (command.length > 0) printHexBytes(", Params=", command.params, command.length);
    else Serial.println();

    CommandType cmdType = static_cast<CommandType>(command.cmd);
    if (cmdType == CommandType::Read) {
      readBuffLen = command.length;
      memset(readBuff, 0, MAX_READ_BUFF_SIZE);
      if (readBuffLen > 0) {
        memcpy(readBuff, command.params, readBuffLen);
      }
    }
    delay(50);
    writer.processMessageQueue();
  });

  //设置发送数据的回调实现
  writer.setWriteCallback([](const byte* buff, byte length) {
    printHexBytes(", bytes=", buff, length);
    //通过串口发送
    commSerial.write(buff, length);
    commSerial.flush();
  });
}

void nm_read_serial() {
  while (commSerial.available()) {
    // 一个一个字节从串口读取
    byte b = commSerial.read();
    parser.onReceivedByte(b);
  }
}

// 检查是否可以读取数据
bool nm_available() {
  return writer.available();
}

void _wait_response(NMCommand command, unsigned long timeout = 7000) {
  unsigned long startTime = millis(); // ms 
  // us, micros(); // 记录开始时间
  // unsigned long elapsedTime;
  Serial.println("wait_response, msgId=" + String(command.msgId) + ", time="+ String(startTime) + ", timeout=" + String(timeout) + "ms"); 

  while (!commSerial.available()) {
    if (millis() - startTime >= timeout) {
      Serial.println("wait_response timeout"); 
      break; // 超时
    }
    // Serial.println("wait_response..."); 
  }

  if (commSerial.available()) {
    nm_read_serial();
    parser.checkReady();
    writer.checkReady();
  }
}

void _get_sensor_values(SensorType sensorType, ReadObjectParamType paramType, uint8_t interface, ValueArrayCallback cb) {  
  // readValueCb = cb;

  if (interface > 5) interface = 0;
  NMCommand command;
  command.cmd = static_cast<uint8_t>(CommandType::Read);
  command.length = 4;
  command.params = new byte[command.length]{ static_cast<uint8_t>(ReadObjectType::Sensor), static_cast<uint8_t>(paramType), static_cast<uint8_t>(sensorType), interface };
  writer.addToMessageQueue(&command);
  _wait_response(command);
}

// interface: A-F, 0-6
bool nm_is_sensor_ready(SensorType sensorType, uint8_t interface) {
  _get_sensor_values(sensorType, ReadObjectParamType::Status, interface, NULL);
  auto is_ready = readBuff[0] != 0;
  Serial.println(strSensorType(sensorType) + " ready status: " + String(is_ready));
  return is_ready;
}

byte* nm_get_sensor_bytes(SensorType sensorType, uint8_t interface) {
  _get_sensor_values(sensorType, ReadObjectParamType::Data, interface, NULL);
  printHexBytes(strSensorType(sensorType) +"=", readBuff, readBuffLen);
  return readBuff;
}

byte nm_get_sensor_byte(SensorType sensorType, uint8_t interface) {
  _get_sensor_values(sensorType, ReadObjectParamType::Data, interface, NULL);
  auto value = readBuff[0];
  Serial.println(strSensorType(sensorType) +"=" + String(value));
  return value;
}

int16_t nm_get_sensor_int16(SensorType sensorType, uint8_t interface) {
  _get_sensor_values(sensorType, ReadObjectParamType::Data, interface, NULL);
  auto value = (readBuff[1] << 8) | readBuff[0]; // 小端，低地址在前
  Serial.println(strSensorType(sensorType) +"=" + String(value));
  return value;
}

byte* nm_get_rgb_values(uint8_t interface) {
  return nm_get_sensor_bytes(SensorType::RGB, interface);
}

byte nm_get_rgb_value(uint8_t interface, uint8_t index) {
  if (index > 2) index = 0;
  nm_get_sensor_bytes(SensorType::RGB, interface);
  auto value = readBuff[index];
  if (index == 0) Serial.println("R=" + String(value));
  else if (index == 1) Serial.println("G=" + String(value));
  else if (index == 2) Serial.println("B=" + String(value));;
  return value;
}

// 手指控制
// position: 0~100, 动作位置百分比
void nm_set_finger(uint8_t finger, uint8_t position) {
  if (position > 100) position = 100;
  NMCommand command;
  command.cmd = static_cast<uint8_t>(CommandType::Control);
  command.length = 3;
  command.params = new byte[command.length]{ static_cast<uint8_t>(ControlType::Finger), finger, position };
  writer.addToMessageQueue(&command);
  _wait_response(command);
}

// 手势控制
// position: 0~100, 动作位置百分比
void nm_set_gesture(uint8_t gesture, uint8_t position) {
  if (position > 100) position = 100;
  NMCommand command;
  command.cmd = static_cast<uint8_t>(CommandType::Control);
  command.length = 3;
  command.params = new byte[command.length]{ static_cast<uint8_t>(ControlType::Gesture), gesture, position };
  writer.addToMessageQueue(&command);
  _wait_response(command);
}

// // LED 控制
// void nm_ledControl(LedNumber ledNum, int r, int g, int b) {
//   // 根据 LED 编号和亮度值执行对应的操作
//   switch (ledNum) {
//     case LedNumber::Led1:
//       // 控制 LED1
//       // ...
//       break;
//     case LedNumber::Led2:
//       // 控制 LED2
//       // ...
//       break;
//     // 可以在此处添加更多的 LED 编号
//     default:
//       break;
//   }
// }

// // 马达控制
// void motorControl(byte* params, int length) {
//   if (length < 2) {
//     // 参数不足，无法执行操作
//     return;
//   }

//   MotorNumber motorNum = static_cast<MotorNumber>(params[0]);
//   uint8_t motorValue = params[1];

//   // 根据马达编号和速度值执行对应的操作
//   switch (motorNum) {
//     case MotorNumber::Motor1:
//       // 控制 Motor1
//       // ...
//       break;
//     case MotorNumber::Motor2:
//       // 控制 Motor2
//       // ...
//       break;
//     // 可以在此处添加更多的马达编号
//     default:
//       break;
//   }
// }

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
String strCommandType(uint8_t value) {
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
String strControlType(uint8_t value) {
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
String strFingerNumber(uint8_t value) {
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
String strGestureNumber(uint8_t value) {
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

// 将枚举类型转换为字符串
String strSensorType(SensorType value) {
  switch (value) {
  // switch (static_cast<SensorType>(value)) {
    case SensorType::SoftSmall:
      return "小柔性传感器";
    case SensorType::Hall:
      return "霍尔传感器";
    case SensorType::RGB:
      return "RGB传感器";
    case SensorType::Infrared:
      return "红外传感器";
    case SensorType::Ultrasonic:
      return "超声波传感器";
    case SensorType::Temperature:
      return "温度传感器";
    case SensorType::Sound:
      return "声音传感器";
    case SensorType::EMG:
      return "肌电信号传感器";
    case SensorType::LED:
      return "LED控制";
    case SensorType::Potentiometer:
      return "旋转电位传感器";
    case SensorType::Button:
      return "Button传感器";
    case SensorType::SoftBig:
      return "大柔性传感器";
    default:
      return "未知传感器";
  }
}

void printHexBytes(String prefix, const byte* buff, int len) {
  Serial.print(prefix);
  for (int i = 0; i < len; i++) {
    Serial.print("0x");
    Serial.print(buff[i], HEX);
    if (i < len - 1) Serial.print(", ");
  }
  Serial.println();
}

/*
// 枚举类型转换为字符串
String enum(GPIOType value) {
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
String enum(GPIOMode value) {
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
String enum(GPIOValue value) {
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
String enum(MotorNumber value) {
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