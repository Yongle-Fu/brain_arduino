#include "nm_command_parser.h"
#include "logger.h"

CommandParser::CommandParser() {}

CommandParser::~CommandParser() {
  if(buffer != NULL) {
    free(buffer);
    buffer = NULL;
  }
  if(startBuffer != NULL) {
    free(startBuffer);
    startBuffer = NULL;
  }
}

bool CommandParser::setBufferSize(unsigned int size) {
  buffer = (byte*)malloc(sizeof(byte) * size);
  if(buffer == NULL) {
    return false;
  }
  bufferMaxSize = size;
  memset(buffer, 0, size);
  return true;
}

void CommandParser::setStart(byte* st, unsigned int size) {
  if(startBuffer != NULL) {
    free(startBuffer);
    startBuffer = NULL;
  }
  startBuffer = (byte*)malloc(sizeof(byte) * size);
  startSize = size;
  memcpy(startBuffer, st, size);
}

bool CommandParser::isMatchStart(byte startOffset) {
  for (int i = 0; i < startSize; i++) {
    if (*(buffer + i + startOffset) != *(startBuffer + i)) {
      return false;
    }
  }
  return true;
}

void CommandParser::setMessageCallback(ResolvedCommandCallback cb) {
  resolvedCommandCallback = cb;
}

void CommandParser::reset() {
  isStart = false;
  bufferLength = 0;
  memset(buffer, 0, bufferMaxSize);
}

void CommandParser::checkReady() {
  reset();
}

void CommandParser::onReceivedByte(byte data) {

  if (Logger::mLevel == DEBUG) {
    if (bufferLength == 0) Serial.print("onReceivedData, bytes=");
    Serial.print("0x");
    Serial.print(data, HEX);
    Serial.print(", ");
  }

  if (!isStart) {
    if (bufferLength == 0 && data != *startBuffer) {
      return;
    }
    *(buffer + bufferLength) = data;
    bufferLength++;
    if (bufferLength >= startSize) {
      //检测到帧头
      bool isMatch = isMatchStart(bufferLength - startSize);
      if (isMatch) {
        isStart = true;
        if (bufferLength > startSize) {
          buffer = buffer + bufferLength - startSize;
          bufferLength = startSize;
        }
      }
    }
    return;
  }

  if (bufferLength >= bufferMaxSize) {
    bufferLength = 0;
    isStart = false;
  }

  *(buffer + bufferLength) = data;
  bufferLength++;

  byte payloadOffset = startSize + 1;
  byte paramOffset = startSize + 4;
  if (isStart && bufferLength >= payloadOffset) {
    byte payloadLength = *(buffer + startSize);
    // 一条指令回复
    if (bufferLength == payloadLength + payloadOffset) {
      // 获取指令回复信息
      byte cmd = *(buffer + payloadOffset);
      byte cmd_attr = *(buffer + payloadOffset + 1);
      byte resultCode = *(buffer + payloadOffset + 2);

      // 判断是否为完整指令（无分包）
      if ((cmd_attr & 0x1) != 0) {
        Serial.println("cmd_attr = " + cmd_attr);
      } else {
        // 判断返回值是否为字节流
        bool isBytes = (cmd_attr & 0x2) == 0;
        // 判断是否同步指令
        bool isSync = (cmd_attr & 0x4) == 0;

        NMCommand command;
        command.cmd = (cmd - 0x80) & 0x7F;
        command.resultCode = resultCode;
        command.isBytes = isBytes;
        command.isSync = isSync;
        command.params = buffer + paramOffset;
        command.length = bufferLength - paramOffset;
        if (resolvedCommandCallback != NULL) {
          resolvedCommandCallback(command);
        }
      }
    
      isStart = false;
      bufferLength = 0;
      memset(buffer, 0, bufferMaxSize);
    }
  }
}