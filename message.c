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

void handleMessage(int epoll_fd, Message *message, char *buf) {

  char buffer[3];

  ssize_t size = read(epoll_fd, buffer, 3);

  char header_flag = buffer[0];

  if (header_flag == GET_MESSAGE) {
    printf("Получение сообщения\n");

    int16_t queue_size = ((uint8_t)buf[1] << 8) | (uint8_t)buf[2];

    char name_queue[256];

    char

        ssize_t size = read(epoll_fd, buffer, 3);

    memcpy(name_queue, &buf[1], 256);

    printf("Название очереди: %s \n", name_queue);

    char tailer[512];

    memcpy(tailer, &buf[256], 512);

    printf("Название потребителя: %s \n", tailer);
  }

  if (header_flag == GET_STAT) {

    printf("Получение статистики\n");

    char name_queue[256];

    memcpy(name_queue, &buf[1], 256);

    printf("Название очереди: %s", name_queue);
  }

  if (header_flag == WRITE_MESSAGE) {
    printf("Запись сообщения:");
  }

  if (header_flag == CHECK_NEW_MESSAGE) {
    printf("Проверка, есть ли новые сообщения в очереди");
  }

  printf("Непонятный заголовок");
}
