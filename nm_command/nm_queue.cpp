#include "nm_queue.h"   

NMQueue::NMQueue() {
  frontIndex = 0;
  rearIndex = -1;
  currentSize = 0;
}

int NMQueue::size() {
  return currentSize;
}

bool NMQueue::isFull() {
  return (currentSize == MAX_QUEUE_SIZE);
}

bool NMQueue::isEmpty() {
  return (currentSize == 0);
}

void NMQueue::enqueue(const NMCommand& command) {
  if (!isFull()) {
    rearIndex = (rearIndex + 1) % MAX_QUEUE_SIZE;
    queueArray[rearIndex] = command;
    currentSize++;
  }
}

NMCommand NMQueue::dequeue() {
  NMCommand command;
  if (!isEmpty()) {
    command = queueArray[frontIndex];
    frontIndex = (frontIndex + 1) % MAX_QUEUE_SIZE;
    currentSize--;
  }
  return command;
}
  
  
