#include <Arduino.h>
#include <SoftwareSerial.h>
#include "nm_command_parser.h"
#include "nm_command_writer.h"
#include "logger.h"


static CommandParser parser;
static CommandWriter writer;

#define MAX_READ_BUFF_SIZE 10
#define LIB_VERSION  "0.0.6"
static int readBuffLen = 0;
static byte readBuff[MAX_READ_BUFF_SIZE];
static ValueArrayCallback readValueCb = NULL;

LogLevel Logger::mLevel = INFO;
// #define hardwareSerialEnabled 0
#ifdef hardwareSerialEnabled
  #define commSerial Serial1  
#else
  // #include <AltSoftSerial.h>
  // AltSoftSerial commSerial(12, 11); // RX, TX
  // SoftwareSerial commSerial(12, 11); // RX, TX
  SoftwareSerial commSerial(19, 18); // RX, TX
#endif

void programSetup() {
  Serial.begin(115200);
  while (!Serial) ; // wait for serial monitor
  commSerial.begin(56000);
  
  Serial.println("\n--------BrainAI start----------");
  Serial.println("Lib ver:" LIB_VERSION);

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
    Logger::print(DEBUG, "msgid=%d, cmd=%d, ret=%d", command.msgId, cmdType, command.resultCode);

    if (!command.isBytes || !command.isSync) {
      Logger::print(DEBUG, "isBytes=%d",command.isBytes);
      Logger::print(DEBUG, "isSync=%d",command.isSync);
    }
    if (command.length > 0) 
      Logger::dumpHex("params=", command.params, command.length);
    else Logger::print(DEBUG, "");

    if (cmdType == CommandType::Read) {
      readBuffLen = command.length;
      memset(readBuff, 0, MAX_READ_BUFF_SIZE);
      if (readBuffLen > 0) {
        memcpy(readBuff, command.params, readBuffLen);
      }
    }
    // delay(50);
    // writer.processMessageQueue();
  });

  //设置发送数据的回调实现
  writer.setWriteCallback([](const byte* buffer, byte length) {
    Logger::dumpHex("bytes", buffer, length);
    //通过串口发送
    commSerial.write(buffer, length);
    commSerial.flush();
  });
}

// 检查是否可以写入
bool isWriterAvailable() {
  return writer.available();
}

// 检查是否可以读取数据
bool isSerivalAvailable() {
  return commSerial.available();
}

void serivalReadBytes() {
  while (isSerivalAvailable()) {
    // 一个一个字节从串口读取
    byte b = commSerial.read();
    parser.onReceivedByte(b);
  }
}

int waitResponse(NMCommand command, unsigned long timeout = 2000) {
  // 记录开始时间
  unsigned long startTime = millis(); // ms, micros() => us
  // unsigned long elapsedTime;
  // Logger::print(DEBUG, "waitResponse, msgId=" + String(command.msgId) + ", time="+ String(startTime) + ", timeout=" + String(timeout) + "ms"); 
  Logger::print(DEBUG, "waitRsp, msgId=%d, time:%d, timeout=%d ms", command.msgId, startTime, timeout);
  while (!isSerivalAvailable()) {
    if (millis() - startTime >= timeout) {
      Logger::print(ERROR, "waitRsp timeout");
      return -1;
    }
    Logger::print(DEBUG,"waitResponse..."); 
  }
  if (isSerivalAvailable()) {
    serivalReadBytes();
    // parser.checkReady();
    // writer.checkReady();
  }
  return 0;
}
void sendCommand(NMCommand command)
{
    int res  = 0;
    uint8_t  sendtry = 2;

    while(sendtry){
      writer.sendCommand(command);
      res = waitResponse(command);
      if (res == 0)
        break;
      sendtry--;
    }
    delete[] command.params;
}

// ******************************************Control*********************************************
void sendControlCommand(NMCommand command) {

  command.cmd = static_cast<uint8_t>(CommandType::Control);
  sendCommand(command);
}

// 手指控制
void setFingerControl(FingerControl* control) {
  NMCommand command;
  command.length = 6;
  command.params = new uint8_t[command.length];
  command.params[0] = static_cast<uint8_t>(control->type);
  for (int i = 0; i < 5; i++) {
    command.params[i + 1] = control->pos[i];
  }
  sendControlCommand(command);
}

// 手势控制
void setGestureControl(GestureControl* control) {
  if (control->pos > 100) control->pos = 100;
  NMCommand command;
  command.length = 3;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->pos
  };
  sendControlCommand(command);
}

void setLedControl(LedControl* control) {
  NMCommand command;
  command.length = 5;
  command.params = new uint8_t[command.length];
  command.params[0] = static_cast<uint8_t>(control->type);
  command.params[1] = static_cast<uint8_t>(control->port);
  for (int i = 0; i < 3; i++) {
    command.params[i + 2] = control->rgb[i];
  }
  sendControlCommand(command);
}

void setMotorControl(MotorControl* control) {
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
  sendControlCommand(command);
}

void setGpioControl(GPIOControl* control) {
  NMCommand command;
  command.length = 4;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    static_cast<uint8_t>(control->level), 
    0
  };
  sendControlCommand(command);
}

void setServoControl(ServoControl* control) {
  NMCommand command;
  command.length = 4;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->angle,
    0
  };
  sendControlCommand(command);
}

void setCarControl(CarControl* control) {
  NMCommand command;
  command.length = 4;
  command.params = new uint8_t[command.length]{ 
    static_cast<uint8_t>(control->type), 
    static_cast<uint8_t>(control->no), 
    control->speed,
    control->time,
  };
  sendControlCommand(command);
}

// ******************************************Read Value******************************************
void sendReadCommand(NMCommand command) {
  command.cmd = static_cast<uint8_t>(CommandType::Read);
  sendCommand(command);
}

void getSensorValues(SensorType sensorType, ReadObjectParamType paramType, InterfaceCode interface) {  
  NMCommand command;
  command.length = 4;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::Sensor), 
    static_cast<uint8_t>(paramType), 
    static_cast<uint8_t>(sensorType), 
    static_cast<uint8_t>(interface), 
  };
  sendReadCommand(command);
}

bool isSensorReady(SensorType sensorType, InterfaceCode interface) {
  getSensorValues(sensorType, ReadObjectParamType::Status, interface);
  bool isReady = readBuff[0] != 0;
  return isReady;
}

// 1 byte, OFF-ON
bool isSensorOn(SensorType sensorType, InterfaceCode interface) {
  if (sensorType != SensorType::Hall && 
      sensorType != SensorType::Infrared &&  
      sensorType != SensorType::Button) {
    return false;
  }
  getSensorValues(sensorType, ReadObjectParamType::Data, interface);
  auto is_on = readBuff[0] != 0;
  return is_on;
}

uint8_t getSensorByte(SensorType sensorType, InterfaceCode interface) {
  if (sensorType != SensorType::Sound) {
    Serial.println("sensorType is not a byte sensor");
    return false;
  }
  getSensorValues(sensorType, ReadObjectParamType::Data, interface);
  auto value = readBuff[0];
  return value;
}

uint8_t* getSensorBytes(SensorType sensorType, InterfaceCode interface) {
  getSensorValues(sensorType, ReadObjectParamType::Data, interface);
  Logger::dumpHex(strSensorType(sensorType).c_str(), readBuff, readBuffLen);
  return readBuff;
}

int16_t getBufferInt16() {
  return (readBuff[1] << 8) | readBuff[0]; // 小端，低地址在前
}

// 2 bytes
// 超声波	5	Ultrasonic <uint16>   [0-1000] (0.1cm)
// 温度	6	Temperature	 <int16>    [-200-1000] (0.1℃)
int16_t getSensorInt16(SensorType sensorType, InterfaceCode interface) {
  if (sensorType != SensorType::SoftSmall &&
      sensorType != SensorType::Ultrasonic &&
      sensorType != SensorType::Temperature && 
      sensorType != SensorType::EMG &&
      sensorType != SensorType::Potentiometer) {
    return 0;
  }
  getSensorValues(sensorType, ReadObjectParamType::Data, interface);
  int16_t value = getBufferInt16();
  Serial.println(strSensorType(sensorType) + ": " + String(value));
  return value;
}

uint8_t getRgbValue(InterfaceCode interface, uint8_t index) {
  if (index > 2) index = 0;
  getSensorBytes(SensorType::RGB, interface);
  auto value = readBuff[index];
  return value;
}

uint8_t* getRgbValues(InterfaceCode interface) {
  return getSensorBytes(SensorType::RGB, interface);
}

// Digital, [1-15]
bool isGpioHigh(uint8_t no) {
  if (no < 1 || no > 15) {
    return 0;
  }
  NMCommand command;
  command.length = 3;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::GPIO), 
    static_cast<uint8_t>(ReadObjectParamType::Data), 
    no
  };
  sendReadCommand(command);
  bool value = readBuff[0];
  return value == 1;
}

// no,  [1-10]
uint16_t getAioValue(uint8_t no) {
  if (no < 1 || no > 10) {
    return 0;
  }
  NMCommand command;
  command.length = 3;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::AIO), 
    static_cast<uint8_t>(ReadObjectParamType::Data), 
    no
  };
  sendReadCommand(command);
  uint16_t value = static_cast<uint16_t>(getBufferInt16());
  return value;
}

uint8_t getFingerValue(FingerNumber no) {
  NMCommand command;
  command.length = 2;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::Finger), 
    static_cast<uint8_t>(no), 
  };
  sendReadCommand(command);
  auto value = readBuff[0];
  return value;
}

uint8_t getIrKey() {
  NMCommand command;
  command.length = 1;
  command.params = new byte[command.length]{ 
    static_cast<uint8_t>(ReadObjectType::IRKey),
  };
  sendReadCommand(command);
  auto value = readBuff[0];
  return value;
}

bool isIrKeyPressed(uint8_t key) {
  return getIrKey() == key;
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
    case GestureNumber::Scissors:
      return "Scissors";
    case GestureNumber::Rock:
      return "Rock";
    case GestureNumber::Paper:
      return "Paper";  
      default:
      return "";
  }
}

void printHexBytes(String prefix, const byte* buffer, int len) {
  Serial.print(prefix);
  for (int i = 0; i < len; i++) {
    Serial.print("0x");
    Serial.print(buffer[i], HEX);
    if (i < len - 1) Serial.print(", ");
  }
  Logger::print(DEBUG, "\n");
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