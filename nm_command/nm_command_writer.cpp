#include "nm_command_writer.h"
#include "nm_queue.h"

static NMQueue messageQueue; // 消息队列

CommandWriter::CommandWriter() {}
CommandWriter::~CommandWriter() {
  if(startBuff != NULL) {
    free(startBuff);
    startBuff = NULL;
  }
}

void CommandWriter::setStart(byte* st, unsigned int size) {
  if(startBuff != NULL) {
    free(startBuff);
    startBuff = NULL;
  }
  startBuff = (byte*)malloc(sizeof(byte) * size);
  startSize = size;
  memcpy(startBuff, st, size);
}

// 将NMCommand结构体添加到消息队列
void CommandWriter::addToMessageQueue(NMCommand* command) {
  command->isBytes = true;
  command->isSync = true;
  command->msgId = msgId;
  msgId++;

  NMCommand& commandRef = *command;
  if (sendingMsgId >= 0) {
    messageQueue.enqueue(commandRef);
  } else {
    sendCommand(commandRef);
  }
}

// 从消息队列中获取并处理NMCommand结构体消息
void CommandWriter::processMessageQueue() {
  sendingMsgId = -1;
  if (!messageQueue.isEmpty()) {
    NMCommand command = messageQueue.dequeue();
    sendCommand(command);
  }
}

void CommandWriter::checkReady() {
  if (sendingMsgId < 0 && messageQueue.isEmpty()) {
    return;
  }
  processMessageQueue();
}

void CommandWriter::setWriteCallback(ValueArrayCallback cb) {
  writeCallback = cb;
}

bool CommandWriter::available() {
  return sendingMsgId < 0 && messageQueue.isEmpty();
}

void CommandWriter::sendCommand(NMCommand& command) {
  if(writeCallback == NULL) {
    return;
  }
  
  uint8_t payload_len = command.length + 2;
  uint8_t len = payload_len + startSize + 1;
  byte data[len];
  for (int i = 0; i < len; i++) {
      if (i < startSize) data[i] = startBuff[i];
      else if (i == startSize) data[i] = payload_len;
      else if (i == startSize+1) data[i] = command.cmd;
      else if (i == startSize+2) data[i] = 0; //cmd_attr
      else data[i] = command.params[i-startSize-3];
  }

  sendingMsgId = command.msgId;
  
  CommandType cmdType = static_cast<CommandType>(command.cmd);
  switch (cmdType) {
    case CommandType::Control: {
      uint8_t ctrlVal = command.params[0];
      ControlType ctrlType = static_cast<ControlType>(ctrlVal);
      switch (ctrlType) {
        case ControlType::Finger:
          Serial.print("\nWrite, msgId: " + String(sendingMsgId) + ", control: Finger, payload_len: " + String(payload_len));
          break;
        case ControlType::Gesture: {
          uint8_t numVal = command.params[1];
          uint8_t posVal = command.params[2];
          Serial.print("\nWrite, msgId: " + String(sendingMsgId) + ", control: Gesture" + strGestureNumber(static_cast<GestureNumber>(numVal)) + ", position: " + String(posVal) + ", payload_len: " + String(payload_len));
          break;
        }
        default:
          Serial.print("\nWrite, msgId: " + String(sendingMsgId) + ", control: " + strControlType(ctrlType) + ", payload_len: " + String(payload_len));
          break;
      }
      break;
    }
    default:
      Serial.print("\nWrite, msgId: " + String(sendingMsgId) + ", cmd: " + strCommandType(cmdType) + ", payload_len: " + String(payload_len));
      break;
  }
  
  writeCallback(data, len);

  // 释放动态分配的内存
  delete[] command.params;
}
