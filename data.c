#include "data.h"
#include "message.h"
#include "utils.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_SEGMENT_SIZE (512 * 1024 * 1024)
#define DATA_FILE_PREFIX "data-"
#define INDEX_DATA_FILE "index.idx"
#define DATA_FILE_EXTENSION ".dat"
#define METADATA_FILE "metadata.mt"

int lock_file_by_fd(int fd, int start, int len, short lock_type) {
  struct flock fl = {
      .l_type = lock_type, .l_whence = SEEK_SET, .l_start = 0, .l_len = len};

  return fcntl(fd, F_SETLKW, &fl);
}

void push(const char *file_name, const unsigned char *data, int size) {
  char metadata_path[512];
  snprintf(metadata_path, sizeof(metadata_path), "%s/%s/%s",
           get_current_directory(), file_name, METADATA_FILE);
  printf("%s", metadata_path);

  int metadata_fd = open(metadata_path, O_CREAT | O_WRONLY | O_EXCL);
  if (metadata_fd > 0) {
    const Metadata metadata = {1, 0, 1, 0};
    uint8_t buffer[20];
    serializeMetadata(&metadata, buffer);
    lock_file_by_fd(metadata_fd, 0, 0, F_WRLCK);
    write(metadata_fd, buffer, 20);
    char segment_id[32];
    uint32_t segment = 1;
    snprintf(segment_id, sizeof(segment_id), "data-%07u.dat", segment);

    char message_data_path[512];
    snprintf(message_data_path, sizeof(message_data_path), "%s/%s/%s",
             get_current_directory(), file_name, segment_id);

    int data_fd = open(message_data_path, O_CREAT | O_WRONLY);

    // TO_DO запись данных в файл

    char index_data_path[512];
    snprintf(index_data_path, sizeof(index_data_path), "%s/%s/%s",
             get_current_directory(), file_name, INDEX_DATA_FILE);

    // TO_DO запись данных в index

  } else {
    Metadata existMetadata = {};
    uint8_t buffer[20];
    read(metadata_fd, buffer, 20);
    deserializeMetadata(buffer, &existMetadata);

    int newSegment = 0;

    if (MAX_SEGMENT_SIZE - existMetadata.currentOffsetWrite < sizeof(data)) {
      newSegment = 1;
    }

    if (newSegment) {
    }

    existMetadata.countMessage = existMetadata.countMessage + 1;
    if (MAX_SEGMENT_SIZE - existMetadata.currentOffsetWrite > sizeof(data)) {
      existMetadata.currentOffsetWrite =
          existMetadata.currentOffsetWrite + 4 + sizeof(data);

    } else {
      existMetadata.currentOffsetWrite = 4 + sizeof(data);
      existMetadata.currentSegment = existMetadata.currentSegment + 1;
      existMetadata.countSegment = existMetadata.countSegment + 1;
    }

    serializeMetadata(&existMetadata, buffer);
    lock_file_by_fd(metadata_fd, 0, 0, F_WRLCK);
    write(metadata_fd, buffer, 20);
  }
}
