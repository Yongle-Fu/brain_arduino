#include "nm_command_parser.h"

CommandParser::CommandParser() {}

CommandParser::~CommandParser() {
  if(buff != NULL) {
    free(buff);
    buff = NULL;
  }
  if(startBuff != NULL) {
    free(startBuff);
    startBuff = NULL;
  }
}

bool CommandParser::setBufferSize(unsigned int size) {
  buff = (byte*)malloc(sizeof(byte) * size);
  if(buff == NULL) {
    return false;
  }
  buffMaxSize = size;
  memset(buff, 0, size);
  return true;
}

void CommandParser::setStart(byte* st, unsigned int size) {
  if(startBuff != NULL) {
    free(startBuff);
    startBuff = NULL;
  }
  startBuff = (byte*)malloc(sizeof(byte) * size);
  startSize = size;
  memcpy(startBuff, st, size);
}

bool CommandParser::isMatchStart(byte startOffset) {
  for (int i = 0; i < startSize; i++) {
    if (*(buff + i + startOffset) != *(startBuff + i)) {
      return false;
    }
  }
  return true;
}

void CommandParser::setResolveCommandCallback(void (*cb)(const NMCommand& command)) {
  this->resolveCommandCallback = cb;
}

void CommandParser::onReceiveData(byte data) {
  if (!isStart) {
    if (buffLen == 0 && data != *startBuff) {
      return;
    }
    *(buff + buffLen) = data;
    buffLen++;
    if (buffLen >= startSize) {
      //检测到帧头
      bool isMatch = isMatchStart(buffLen - startSize);
      if (isMatch) {
        isStart = true;
        if (buffLen > startSize) {
          buff = buff + buffLen - startSize;
          buffLen = startSize;
        }
      }
    }
    return;
  }

  if (buffLen >= buffMaxSize) {
    buffLen = 0;
    isStart = false;
  }

  *(buff + buffLen) = data;
  buffLen++;

  byte payload_offset = startSize + 1;
  byte params_offset = startSize + 4;
  if (isStart && buffLen >= payload_offset) {
    byte payload_len = *(buff + startSize);
    // 一条指令回复
    if (buffLen == payload_len + payload_offset) {
      // 获取指令回复信息
      byte cmd = *(buff + payload_offset);
      byte cmd_attr = *(buff + payload_offset + 1);
      byte resultCode = *(buff + payload_offset + 2);

      // 判断是否为完整指令（无分包）
      if ((cmd_attr & 0x1) == 0) {
        // 判断返回值是否为字节流
        bool isBytes = (cmd_attr & 0x2) == 0;
        // 判断是否同步指令
        bool isSync = (cmd_attr & 0x4) == 0;
        if (resolveCommandCallback != NULL) {
          NMCommand command;
          command.cmd = (cmd - 0x80) & 0x7F;
          command.resultCode = resultCode;
          command.isBytes = isBytes;
          command.isSync = isSync;
          command.params = buff + params_offset;
          command.length = buffLen - params_offset;
          resolveCommandCallback(command);

          isStart = false;
          buffLen = 0;
          memset(buff, 0, buffMaxSize);
        }
      }
    }
  }
}