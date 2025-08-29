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
    push_new_metadata(metadata_fd, file_name, data, size);
  } else {
    metadata_fd = open(metadata_path, O_CREAT | O_WRONLY);
    push_existing_metadata(metadata_fd, file_name, data, size);
  }
}

void push_new_metadata(int metadata_fd, const char *path,
                       const unsigned char *data, int size) {

  const Metadata metadata = {1, 0, 1, 0};
  uint8_t buffer[20];
  serializeMetadata(&metadata, buffer);
  lock_file_by_fd(metadata_fd, 0, 0, F_WRLCK);
  write(metadata_fd, buffer, 20);
  char segment_id[32];
  uint32_t segment = 1;
  snprintf(segment_id, sizeof(segment_id), "data-%07u.dat", segment);

  FilePaths file_path = {};

  build_file_paths(path, segment, &file_path);

  int data_fd = open(file_path.segment_path, O_CREAT | O_WRONLY);

  // TO_DO запись данных в файл
}

void build_file_paths(const char *base_dir, uint32_t segment,
                      FilePaths *paths) {
  snprintf(paths->metadata_path, sizeof(paths->metadata_path), "%s/%s",
           base_dir, METADATA_FILE);

  snprintf(paths->segment_path, sizeof(paths->segment_path), "%s/data-%07u.dat",
           base_dir, segment);

  snprintf(paths->index_path, sizeof(paths->index_path), "%s/%s", base_dir,
           INDEX_DATA_FILE);
}

void push_existing_metadata(int metadata_fd, const char *path,
                            const unsigned char *data, int size) {

  Metadata existMetadata = {};
  lock_file_by_fd(metadata_fd, 0, 0, F_WRLCK);

  uint8_t buffer[20];
  read(metadata_fd, buffer, 20);
  deserializeMetadata(buffer, &existMetadata);

  int isNewSegment = 0;

  uint8_t time[8];

  getBufferAtTime(time);

  if (MAX_SEGMENT_SIZE - (existMetadata.currentOffsetWrite + 8) <
      sizeof(data)) {
    isNewSegment = 1;
  }

  char segment_id[32];
  uint32_t segment = existMetadata.countSegment;

  if (isNewSegment) {
    segment = segment + 1;
  }

  FilePaths file_path = {};

  build_file_paths(path, segment, &file_path);

  snprintf(segment_id, sizeof(segment_id), "data-%07u.dat", segment);

  int data_file_fd = open(file_path.segment_path, O_APPEND | O_WRONLY);

  uint32_t size_data = sizeof(data);
  uint8_t size_data_buffer[4];
  write_u32_be(size_data_buffer, size_data);

  write(data_file_fd, size_data_buffer, sizeof(size_data_buffer));
  write(data_file_fd, time, sizeof(time));
  write(data_file_fd, data, sizeof(data));

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
