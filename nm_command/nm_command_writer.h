/*
 * 自定义通信协议发送
 * 支持arduino uno
 * 发送实际数据内容长度最多255个数据, 1 byte
 * @Author: Yongle Fu
 * @Date: 2023-05-12
 * version: 0.0.1
 */
#ifndef NM_Command_Write_H_
#define NM_Command_Write_H_

#include <Arduino.h>
#include "nm_command.h"

class CommandWriter {
private:
  byte* startBuff = NULL;
  int startSize = 0;
  void (*writeCallback)(byte* buff, byte length) = NULL;

  int msgId = 0;
  bool sending = false;
  // 发送指令
  void sendCommand(const NMCommand& command);

public:
  CommandWriter();
  ~CommandWriter();

  // 设置帧头
  void setStart(byte* st, unsigned int size);

  // 将NMCommand结构体添加到消息队列
  void addToMessageQueue(NMCommand* command);

  // 从消息队列中获取并处理NMCommand结构体消息
  void processMessageQueue();

  // 设置发送数据的回调实现方式, 比如通过串口发送,i2c,spi发送等
  void setWriteCallback(void (*cb)(byte* buff, byte length));
};

#endif