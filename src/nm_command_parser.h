/*
 * 自定义通信协议解析
 * 支持arduino uno
 * @Author: Yongle Fu
 * @Date: 2023-05-12
 * version: 0.0.1
 */
#ifndef NM_Command_Parser_H_
#define NM_Command_Parser_H_

#include <Arduino.h>
#include "nm_command.h"

typedef void (*ResolvedCommandCallback)(NMCommand& command); 

class CommandParser {
  private:
  bool isStart = false;
  byte* startBuffer = NULL;
  int startSize = 0;

  byte* buffer = NULL;
  int bufferLength = 0;
  int bufferMaxSize = 0;
  
  ResolvedCommandCallback resolvedCommandCallback = NULL;
  
  bool isMatchStart(byte startOffset);
  void reset();

  public:
  CommandParser();
  ~CommandParser();

  //设置帧头
  void setStart(byte* st, unsigned int size);

  //设置缓冲大小
  bool setBufferSize(unsigned int size);

  //设置解析到数据的回调
  void setMessageCallback(ResolvedCommandCallback cb);
  
  //往里添加数据进行解析
  void onReceivedByte(byte data);

  void checkReady();
};
#endif