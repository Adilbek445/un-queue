#include "data.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// флаги в заголовке сообщения
#define GET_MESSAGE 0x1
#define WRITE_MESSAGE 0x2
#define GET_STAT 0x3
#define CHECK_NEW_MESSAGE 0x4

#define BUFFER_SIZE 1024

char current_dir[128];

void handleMessage(int epoll_fd) {

  init_current_directory();
  uint8_t header[7];
  ssize_t headerSize = read(epoll_fd, header, sizeof(header));

  if (headerSize != sizeof(header)) {
    fprintf(stderr,
            "Ошибка: не удалось прочитать полный заголовок (%zd байт)\n",
            headerSize);
    return;
  }

  char header_flag = header[0];

  uint16_t queue_name_size = (header[1] << 8) | header[2];
  uint32_t tailer_or_payload_size =
      (header[3] << 24) | (header[4] << 16) | (header[5] << 8) | header[6];

  printf("queue_name_size %u\n", queue_name_size);
  printf("tailer_or_payload_size %u\n", tailer_or_payload_size);

  char *name_queue = NULL;

  char name_queue_stack[512];

  if (header_flag == WRITE_MESSAGE)
    name_queue = malloc(queue_name_size + tailer_or_payload_size);
  else
    name_queue = name_queue_stack;

  char tailer[tailer_or_payload_size];

  ssize_t size =
      read(epoll_fd, name_queue, queue_name_size + tailer_or_payload_size);

  memcpy(tailer, name_queue + queue_name_size, tailer_or_payload_size);

  name_queue[queue_name_size] = '\0';
  tailer[tailer_or_payload_size] = '\0';

  printf("Название очереди: %s \n", name_queue);
  printf("Название потребителя: %s \n", tailer);

  char directory[512];

  stpcpy(directory, get_current_directory());
  stpcpy(directory, name_queue);

  if (header_flag == GET_MESSAGE) {
    printf("Получение сообщения\n");
    char text[] = "Hello World";
    write(epoll_fd, text, 11);

    close(epoll_fd);
    return;
  }

  if (header_flag == GET_STAT) {
    printf("Получение статистики\n");
    close(epoll_fd);
    return;
  }

  if (header_flag == WRITE_MESSAGE) {
    printf("Запись сообщения: \n");

    if (!is_directory_exist(directory)) {
      mkdir(name_queue, 0755);
    }
    push(name_queue, tailer, tailer_or_payload_size);
    close(epoll_fd);
    free(name_queue);
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

void init_current_directory() {
  if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    perror("getcwd");
  }
}

const char *get_current_directory() { return current_dir; }
