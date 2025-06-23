#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_SEGMENT_SIZE (512 * 1024 * 1024)
#define DATA_FILE_PREFIX "data-";
#define INDEX_DATA_PREFIX "index-"
#define DATA_FILE_EXTENSION ".dat";
#define INDEX_FILE_EXTENSION ".idx";
#define METADATA_FILE "metadata.mt"

// флаги в заголовке сообщения
#define GET_MESSAGE 0x1
#define WRITE_MESSAGE 0x2
#define GET_STAT 0x3
#define CHECK_NEW_MESSAGE 0x4

#define MAX_EVENTS 1
#define BUFFER_SIZE 1024
char current_dir[128];

int lock_file_by_fd(int fd, int start, int len, int lock_type);

#pragma pack(push, 1)
typedef struct {
  uint32_t size;
  uint64_t time;

} MessageData;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t currentSegment;
  uint64_t currentOffsetWrite;
  uint8_t countSegment;
  uint32_t countMessage;
} Metadata;
#pragma pack(pop)

typedef struct {
  uint32_t segmentId;
  uint64_t offsetInData;
} IndexData;

void init_current_directory() {
  if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    perror("getcwd");
  }
}

const char *get_current_directory() { return current_dir; }

int main(int argc, char *argv[]) {

  init_current_directory();

  int channel_fd = open(argv[1], O_RDONLY);

  int epoll_fd = epoll_create1(0);

  struct epoll_event event;
  event.events = EPOLLIN | EPOLLHUP;

  event.data.fd = channel_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, channel_fd, &event);

  char buf[BUFFER_SIZE];

  int server_running = 1;

  while (server_running) {

    struct epoll_event events[MAX_EVENTS];
    int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (int i = 0; i < num_events; i++) {
      int fd = events[i].data.fd;

      ssize_t size = read(fd, buf, BUFFER_SIZE);

      char header_flag = buf[0];

      if (header_flag == GET_MESSAGE) {
        printf("Получение сообщения\n");
        char name_queue[256];
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
    }
  }

  int fd = open("/home/adilbek445/projects/c-lang/un-queue/", O_RDWR | O_TRUNC);
  Metadata q = {8, 32, 11, 256};
  Metadata w = {0, 0, 0, 0};

  char metadataFile[100] = "";
  snprintf(metadataFile, sizeof(metadataFile), "%s%s", current_dir,
           METADATA_FILE);

  int metadataFd = open(metadataFile, O_RDWR);
  lock_file_by_fd(metadataFd, 0, 16, F_SETLKW);

  write(fd, &q, sizeof(q));
  lseek(fd, 0, SEEK_SET);

  printf("%s\n", current_dir);

  ssize_t bytesRead = read(fd, &w, sizeof(w));
  printf("sizeof(char) = %zu\n", sizeof(char));
  close(fd);

  printf("Размер Metadata w = %zu\n", bytesRead);

  printf("currentSegment: %u\n", w.currentSegment);
  printf("currentOffsetWrite: %lu\n", w.currentOffsetWrite);
  printf("countSegment: %u\n", w.countSegment);
  printf("countMessage: %u\n", w.countMessage);
}

// блокировка конца файла
int lock_file_by_fd(int fd, int start, int len, int lock_type) {
  struct flock fl = {
      .l_type = lock_type,  // write‑lock (эксклюзивная)
      .l_whence = SEEK_SET, // от начала файла
      .l_start = 0,         // смещение
      .l_len = len          // 0 = «до конца файла» → весь файл
  };

  /* F_SETLKW = «ждать, пока не получится поставить блокировку» */
  return fcntl(fd, F_SETLKW, &fl);
}

void putData(const char *file_name, const unsigned char *data, int size) {}

void getData(const char *file_name, unsigned char *data) {}

void lockfile(int fd) { struct flock fl = {0}; }
