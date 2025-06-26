#include "message.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// флаги в заголовке сообщения
#define GET_MESSAGE 0x1
#define WRITE_MESSAGE 0x2
#define GET_STAT 0x3
#define CHECK_NEW_MESSAGE 0x4

#define BUFFER_SIZE 1024

void handleMessage(int epoll_fd) {

  MessageHeader message_header = {0, 0, 0};
  ssize_t size = read(epoll_fd, &message_header, sizeof(message_header));
  char header_flag = message_header.command_type;
  uint16_t queue_name_size = message_header.queue_name_len;
  uint32_t tailer_or_payload_size = message_header.tailer_size;

  printf("queue_name_size %u\n", queue_name_size);

  printf("tailer_or_payload_size %u\n", tailer_or_payload_size);

  if (header_flag == GET_MESSAGE) {
    printf("Получение сообщения\n");

    char name_queue[queue_name_size + tailer_or_payload_size];
    char tailer[tailer_or_payload_size + 1];

    ssize_t size = read(epoll_fd, name_queue, queue_name_size);

    memcpy(tailer, name_queue + queue_name_size, tailer_or_payload_size);

    name_queue[queue_name_size + 1] = '\0';

    printf("Название очереди: %s \n", name_queue);

    printf("Название потребителя: %s \n", tailer);
  }

  if (header_flag == GET_STAT) {

    printf("Получение статистики\n");
  }

  if (header_flag == WRITE_MESSAGE) {
    printf("Запись сообщения:");
  }

  if (header_flag == CHECK_NEW_MESSAGE) {
    printf("Проверка, есть ли новые сообщения в очереди");
  }

  printf("Непонятный заголовок");
}
