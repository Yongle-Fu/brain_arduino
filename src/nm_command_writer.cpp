#include "nm_command_writer.h"
#include "nm_queue.h"
#include "logger.h"

static NMQueue messageQueue; // 消息队列

CommandWriter::CommandWriter() {}
CommandWriter::~CommandWriter() {
  if(startBuffer != NULL) {
    free(startBuffer);
    startBuffer = NULL;
  }
}

void CommandWriter::setStart(byte* st, unsigned int size) {
  if(startBuffer != NULL) {
    free(startBuffer);
    startBuffer = NULL;
  }
  startBuffer = (byte*)malloc(sizeof(byte) * size);
  startSize = size;
  memcpy(startBuffer, st, size);
}

// 将NMCommand结构体添加到消息队列
void CommandWriter::addToMessageQueue(NMCommand& command) {
  command.isBytes = true;
  command.isSync = true;
  command.msgId = msgId;
  msgId++;
  
  if (sendingMsgId >= 0) {
    messageQueue.enqueue(command);
  } else {
    sendCommand(command);
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
  
  uint8_t payloadLength = command.length + 2;
  uint8_t len = payloadLength + startSize + 1;
  byte data[len];
  for (int i = 0; i < len; i++) {
      if (i < startSize) data[i] = startBuffer[i];
      else if (i == startSize) data[i] = payloadLength;
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
          Logger::print(DEBUG, "wr,msgid:%d, ctr:finger, payload len:%d",sendingMsgId, payloadLength );
          break;
        case ControlType::Gesture: {
          uint8_t numVal = command.params[1];
          uint8_t posVal = command.params[2];
          Logger::print(DEBUG, "wr,msgid:%d, ctr:gesture, %s,pos:%d,len:%d",sendingMsgId, strGestureNumber(static_cast<GestureNumber>(numVal)).c_str(),posVal, payloadLength);

          break;
        }
        default:
          Logger::print(DEBUG, "wr,msgid:%d, ctr:%s,len:%d",sendingMsgId, strControlType(ctrlType).c_str(),  payloadLength);
          break;
      }
      break;
    }
    default:
          Logger::print(DEBUG, "wr,msgid:%d, cmd:%s,len:%d",sendingMsgId, strCommandType(cmdType).c_str(),  payloadLength);
      break;
  }
  
  writeCallback(data, len);

  // 释放动态分配的内存
  // delete[] command.params;
}
