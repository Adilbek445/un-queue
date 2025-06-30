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
  uint8_t header[7];
  ssize_t headerSize = read(epoll_fd, header, sizeof(header));
  char header_flag = header[0];
  uint16_t queue_name_size = (header[1] << 8) | header[2];
  uint32_t tailer_or_payload_size =
      (header[3] << 24) | (header[4] << 16) | (header[5] << 8) | header[6];

  printf("queue_name_size %u\n", queue_name_size);

  printf("tailer_or_payload_size %u\n", tailer_or_payload_size);

  char name_queue[queue_name_size + tailer_or_payload_size];
  char tailer[tailer_or_payload_size + 1];

  ssize_t size =
      read(epoll_fd, name_queue, queue_name_size + tailer_or_payload_size);

  memcpy(tailer, name_queue + queue_name_size, tailer_or_payload_size);
  name_queue[queue_name_size + 1] = '\0';
  printf("Название очереди: %s \n", name_queue);
  printf("Название потребителя: %s \n", tailer);

  if (header_flag == GET_MESSAGE) {
    printf("Получение сообщения\n");

    close(epoll_fd);

    return;
  }

  if (header_flag == GET_STAT) {
    printf("Получение статистики\n");
    close(epoll_fd);
    return;
  }

  if (header_flag == WRITE_MESSAGE) {
    printf("Запись сообщения:");
    close(epoll_fd);
    return;
  }

  if (header_flag == CHECK_NEW_MESSAGE) {
    printf("Проверка, есть ли новые сообщения в очереди");
    close(epoll_fd);
    return;
  }

  printf("Непонятный заголовок\n");
  close(epoll_fd);
}
