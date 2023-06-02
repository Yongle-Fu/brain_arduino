#include <Arduino.h>
#include <SoftwareSerial.h>
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
  // #include <AltSoftSerial.h>
  // AltSoftSerial commSerial(12, 11); // RX, TX
  // SoftwareSerial commSerial(12, 11); // RX, TX
  SoftwareSerial commSerial(19, 18); // RX, TX
#endif

void nm_setup() {
  Serial.begin(115200);
  while (!Serial) ; // wait for serial monitor
  commSerial.begin(56000);

#ifdef hardwareSerialEnabled
  pinMode(PB2, OUTPUT);
  pinMode(PB3, INPUT_PULLUP);
#endif  

  Serial.println("\n---------setup---------");
  Serial.println("Build: 2023/06/02-17:00");

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
    CommandType cmdType = static_cast<CommandType>(command.cmd);
    Serial.print("msgId=" + String(command.msgId) + ", cmd=" + strCommandType(cmdType));
    // if (cmdType == CommandType::Control) {
    //   auto ctrl = command.params[0];
    //   Serial.print(", contorl=" + strControlType(static_cast<ControlType>(ctrl)));
    // }
    Serial.print(", retCode=" + String(command.resultCode));
    if (!command.isBytes || !command.isSync) {
      Serial.print(", isBytes=" + String(command.isBytes));
      Serial.print(", isSync=" + String(command.isSync));
    }
    if (command.length > 0) printHexBytes(", Params=", command.params, command.length);
    else Serial.println();

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

// 检查是否可以读取数据
bool nm_write_available() {
  return writer.available();
}

bool nm_read_available() {
  return commSerial.available();
}

void nm_serial_read() {
  while (nm_read_available()) {
    // 一个一个字节从串口读取
    byte b = commSerial.read();
    parser.onReceivedByte(b);
  }
}

void wait_response(NMCommand command, unsigned long timeout = 7000) {
  // 记录开始时间
  unsigned long startTime = millis(); // ms, micros() => us
  // unsigned long elapsedTime;
  Serial.println("wait_response, msgId=" + String(command.msgId) + ", time="+ String(startTime) + ", timeout=" + String(timeout) + "ms"); 

  while (!nm_read_available()) {
    if (millis() - startTime >= timeout) {
      Serial.println("wait_response timeout"); 
      break; // 超时
    }
    // Serial.println("wait_response..."); 
  }

  if (nm_read_available()) {
    nm_serial_read();
    parser.checkReady();
    writer.checkReady();
  }
}

// ******************************************Control*********************************************
void send_control_command(NMCommand command) {
  command.cmd = static_cast<uint8_t>(CommandType::Control);
  writer.addToMessageQueue(&command);
  wait_response(command);
}

// 手指控制
void nm_set_finger(FingerControl* control) {
  NMCommand command;
  command.length = 6;
  command.params = new uint8_t[command.length];
  command.params[0] = static_cast<uint8_t>(control->type);
  for (int i = 0; i < 5; i++) {
    command.params[i + 1] = control->pos[i];
  }
  send_control_command(command);
}

// 手势控制
void nm_set_gesture(GestureControl* control) {
  if (control->pos > 100) control->pos = 100;
  NMCommand command;
  command.length = 3;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->pos
  };
  send_control_command(command);
}

void nm_set_led(LedControl* control) {
  NMCommand command;
  command.length = 5;
  command.params = new uint8_t[command.length];
  command.params[0] = static_cast<uint8_t>(control->type);
  command.params[1] = static_cast<uint8_t>(control->no);
  for (int i = 0; i < 3; i++) {
    command.params[i + 2] = control->rgb[i];
  }
  send_control_command(command);
}

void nm_set_motor(MotorControl* control) {
  NMCommand command;
  command.length = 6;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->direction,
    control->speed,
    control->angle,
    control->time,  
  };
  send_control_command(command);
}

void nm_set_gpio(GPIOControl* control) {
  NMCommand command;
  command.length = 4;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    static_cast<uint8_t>(control->level), 
    0
  };
  send_control_command(command);
}

void nm_set_servo(ServoControl* control) {
  NMCommand command;
  command.length = 4;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->angle,
    0
  };
  send_control_command(command);
}

void nm_set_car(CarControl* control) {
  NMCommand command;
  command.length = 4;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->speed,
    control->time,
  };
  send_control_command(command);
}

// ******************************************Read Value******************************************
void send_read_command(NMCommand command) {
  command.cmd = static_cast<uint8_t>(CommandType::Read);
  writer.addToMessageQueue(&command);
  wait_response(command);
}

void get_sensor_values(SensorType sensorType, ReadObjectParamType paramType, InterfaceCode interface) {  
  NMCommand command;
  command.length = 4;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::Sensor), 
    static_cast<uint8_t>(paramType), 
    static_cast<uint8_t>(sensorType), 
    static_cast<uint8_t>(interface), 
  };
  send_read_command(command);
}

bool nm_is_sensor_ready(SensorType sensorType, InterfaceCode interface) {
  get_sensor_values(sensorType, ReadObjectParamType::Status, interface);
  auto is_ready = readBuff[0] != 0;
  Serial.println(strSensorType(sensorType) + ", ready status: " + String(is_ready));
  return is_ready;
}

// 1 byte, OFF-ON
bool nm_is_sensor_on(SensorType sensorType, InterfaceCode interface) {
  if (sensorType != SensorType::Hall && 
      sensorType != SensorType::Infrared &&  
      sensorType != SensorType::Button) {
    Serial.println("sensorType is not a switch sensor");
    return false;
  }
  get_sensor_values(sensorType, ReadObjectParamType::Data, interface);
  auto is_on = readBuff[0] != 0;
  Serial.println(strSensorType(sensorType) + ", is_on: " + String(is_on));
  return is_on;
}

uint8_t nm_get_sensor_byte(SensorType sensorType, InterfaceCode interface) {
  if (sensorType != SensorType::Sound) {
    Serial.println("sensorType is not a byte sensor");
    return false;
  }
  get_sensor_values(sensorType, ReadObjectParamType::Data, interface);
  auto value = readBuff[0];
  Serial.println(strSensorType(sensorType) + ": " + String(value));
  return value;
}

uint8_t* nm_get_sensor_bytes(SensorType sensorType, InterfaceCode interface) {
  get_sensor_values(sensorType, ReadObjectParamType::Data, interface);
  printHexBytes(strSensorType(sensorType) + ": ", readBuff, readBuffLen);
  return readBuff;
}

int16_t get_buffer_int16() {
  return (readBuff[1] << 8) | readBuff[0]; // 小端，低地址在前
}

// 2 bytes
// 超声波	5	Ultrasonic <uint16>   [0-1000] (0.1cm)
// 温度	6	Temperature	 <int16>    [-200-1000] (0.1℃)
int16_t nm_get_sensor_int16(SensorType sensorType, InterfaceCode interface) {
  if (sensorType != SensorType::SoftSmall &&
      sensorType != SensorType::Ultrasonic &&
      sensorType != SensorType::Temperature && 
      sensorType != SensorType::EMG &&
      sensorType != SensorType::Potentiometer) {
    Serial.println("sensorType is not a int16 sensor");
    return 0;
  }
  get_sensor_values(sensorType, ReadObjectParamType::Data, interface);
  int16_t value = get_buffer_int16();
  Serial.println(strSensorType(sensorType) + ": " + String(value));
  return value;
}

uint8_t nm_get_rgb_value(InterfaceCode interface, uint8_t index) {
  if (index > 2) index = 0;
  nm_get_sensor_bytes(SensorType::RGB, interface);
  auto value = readBuff[index];
  if (index == 0) Serial.println("R=" + String(value));
  else if (index == 1) Serial.println("G=" + String(value));
  else if (index == 2) Serial.println("B=" + String(value));
  return value;
}

uint8_t* nm_get_rgb_values(InterfaceCode interface) {
  return nm_get_sensor_bytes(SensorType::RGB, interface);
}

// Digital, [1-15]
bool nm_get_gpio(uint8_t no) {
  if (no < 1 || no > 15) {
    Serial.println("gpio no is out of range");
    return 0;
  }
  NMCommand command;
  command.length = 3;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::GPIO), 
    static_cast<uint8_t>(ReadObjectParamType::Data), 
    no
  };
  send_read_command(command);
  auto value = readBuff[0];
  Serial.println("GPIO_" + String(no) + ": " + String(value));
  return value == 1;
}

// no,  [1-10]
uint16_t nm_get_aio(uint8_t no) {
  if (no < 1 || no > 10) {
    Serial.println("Analog no is out of range");
    return 0;
  }
  NMCommand command;
  command.length = 3;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::AIO), 
    static_cast<uint8_t>(ReadObjectParamType::Data), 
    no
  };
  send_read_command(command);
  uint16_t value = static_cast<uint16_t>(get_buffer_int16());
  Serial.println("Analog_" + String(no) + ": " + String(value));
  return value;
}

uint8_t nm_get_finger(FingerNumber no) {
  NMCommand command;
  command.length = 2;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::Finger), 
    static_cast<uint8_t>(no), 
  };
  send_read_command(command);
  auto value = readBuff[0];
  Serial.println(strFingerNumber(no) + ": " + String(value));
  return value;
}

uint8_t nm_get_ir_key() {
  NMCommand command;
  command.length = 1;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::IRKey),
  };
  send_read_command(command);
  auto value = readBuff[0];
  Serial.println("IRKey: " + String(value));
  return value;
}

bool nm_is_ir_key_pressed(uint8_t key) {
  return nm_get_ir_key() == key;
}

// ******************************************Print Methods******************************************
String strCommandType(CommandType type) {
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

String strControlType(ControlType type) {
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
    case ControlType::Car:
      return "Car";  
    default:
      return "Unknown";
  }
}

String strSensorType(SensorType type) {
  switch (type) {
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

String strFingerNumber(FingerNumber no) {
  switch (no) {
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
  }
}

String strGestureNumber(GestureNumber no) {
  switch (no) {
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
String enum(GPIOLevel level) {
  switch (level) {
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