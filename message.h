#include <inttypes.h>
#include <stdio.h>

typedef struct {
  uint8_t action;
  char *queue_name;
  char *consumer_name;
} Message;

#pragma pack(push, 1)
typedef struct {
  uint8_t command_type;
  uint16_t queue_name_len;
  uint32_t tailer_size;
} MessageHeader;
#pragma pack(pop)

void handleMessage(int epoll_fd);
