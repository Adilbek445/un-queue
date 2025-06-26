#include "data.h"
#include "message.h"
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

#define MAX_EVENTS 1
#define BUFFER_SIZE 1024
char current_dir[128];

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

      handleMessage(fd);
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
