#include <inttypes.h>
#include <stdio.h>

typedef struct {
  uint8_t action;
  char *queue_name;
  char *consumer_name;
} Message;

void handleMessage(int epoll_fd, Message *message, char *buf);
