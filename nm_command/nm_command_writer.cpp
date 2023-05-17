#include "nm_command_writer.h"
#include "nm_queue.h"

static NMQueue messageQueue; // 消息队列

CommandWriter::CommandWriter() {
  // messageQueue = NMQueue();
}
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
  Serial.print("command->length : " + String(command->length) + "\n");
  // Serial.print("msgId: " + String(msgId) + "\n");
  // Serial.println(command->params[0], HEX);
  // Serial.println(command->params[1], HEX);
  // Serial.println(command->params[2], HEX);

  const NMCommand& commandRef = *command;
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

void CommandWriter::setWriteCallback(void (*cb)(byte* buff, byte length)) {
  this->writeCallback = cb;
}

bool CommandWriter::isEmptyQueue() {
  return messageQueue.isEmpty();
}

bool CommandWriter::isReadAvailable() {
  return sendingMsgId < 0 && isEmptyQueue();
}

void CommandWriter::sendCommand(const NMCommand& command) {
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
  uint8_t ctrlVal, numVal, posVal;

  switch (cmdType) {
    case CommandType::Control:
      ctrlVal = static_cast<uint8_t>(command.params[0]);
      numVal = static_cast<uint8_t>(command.params[1]);
      posVal = static_cast<uint8_t>(command.params[2]);
      switch (static_cast<ControlType>(ctrlVal)) {
        case ControlType::Finger:
          Serial.println("Write, msgId: " + String(command.msgId) + ", control: " + strControlType(ctrlVal) + strFingerNumber(numVal) + ", position: " + String(posVal));
          break;
        case ControlType::Gesture:
          Serial.println("Write, msgId: " + String(command.msgId) + ", control: " + strControlType(ctrlVal) + strGestureNumber(numVal) + ", position: " + String(posVal));
          break;
        default:
          break;
      }
      break;
    default:
      Serial.println("Write, msgId: " + String(command.msgId) + ", cmd: " + strCommandType(command.cmd) + ", payload_len: " + String(payload_len));
      break;
  }
  
  writeCallback(data, len);

  // 释放动态分配的内存
  delete[] command.params;
}
