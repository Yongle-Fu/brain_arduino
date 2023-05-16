#ifndef NM_QUEUE_H_
#define NM_QUEUE_H_

#include "nm_command.h"   

const int MAX_QUEUE_SIZE = 20; // 消息队列的最大容量

class NMQueue {
private:
  NMCommand queueArray[MAX_QUEUE_SIZE];
  int frontIndex = 0;
  int rearIndex = -1;
  int currentSize = 0;
  
public:
  NMQueue();
  
  int size();
  bool isFull();
  bool isEmpty();
  
  void enqueue(const NMCommand& command);
  NMCommand dequeue();
};

#endif
