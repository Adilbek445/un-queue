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
#define DATA_HEADER_SIZE 12

int lock_file_by_fd(int fd, int start, int len, short lock_type) {
  struct flock fl = {
      .l_type = lock_type, .l_whence = SEEK_SET, .l_start = 0, .l_len = len};

  return fcntl(fd, F_SETLKW, &fl);
}

/**
 * @brief Записывает сообщение в файловую очередь.
 *
 * Проверяет есть файл metadata.mt. Если есть записывает в него сообщение.
 * Если нет создает новый файл.
 *
 * @param queue_name Директория очереди.
 * @param data Данные для записи.
 * @param size Размер данных в байтах.
 */
void push(const char *queue_name, char *data, int size) {
  char metadata_path[512];
  snprintf(metadata_path, sizeof(metadata_path), "%s/%s/%s",
           get_current_directory(), queue_name, METADATA_FILE);
  printf("%s \n", metadata_path);

  int metadata_fd = open(metadata_path, O_CREAT | O_WRONLY | O_EXCL, 0644);
  if (metadata_fd > 0) {
    printf("Запись в новый файл \n");
    push_new_metadata(metadata_fd, queue_name, data, size);

  } else {
    printf("Запись в существующий файл \n");
    metadata_fd = open(metadata_path, O_RDWR);
    push_existing_metadata(metadata_fd, queue_name, data, size);
  }
  close(metadata_fd);
}

/**
 * @brief Записывает данные в новый файл
 *
 * Создаёт новый файл metadata.mt. Собирает данные и записывает в этот файл
 *
 * @param metadata_fd Файловый дескриптор нового файла
 * @param path Путь к очереди.
 * @param data Данные для записи.
 * @param size Размер данных в байтах.
 */
void push_new_metadata(int metadata_fd, const char *path, char *data,
                       int size) {
  printf("Данные в data '%s' \n", data);
  lock_file_by_fd(metadata_fd, 0, 0, F_WRLCK);
  uint32_t segment = 1;

  FilePaths file_path = {};
  build_file_paths(path, segment, &file_path);

  uint8_t time[8];
  writeBufferAtTime(time);

  printf("Путь dataFile: %s \n", file_path.segment_path);

  int data_file_fd =
      open(file_path.segment_path, O_CREAT | O_WRONLY | O_APPEND, 0644);

  write_data_file(data_file_fd, data, size, time);

  int index_file_fd =
      open(file_path.index_path, O_CREAT | O_WRONLY | O_APPEND, 0644);

  IndexData index_struct = {1, 1, size, 0, (uint64_t)time};

  write_index_file(index_file_fd, &index_struct);

  int currentOffsetWrite = 4 + sizeof(time) + size;

  printf("sizeof(time) : '%u' \n", sizeof(time));
  printf("sizeof(data) : '%u' \n", size);

  printf("Размер сдвига в data файле: '%u' \n", currentOffsetWrite);

  const Metadata metadata = {1, currentOffsetWrite, 1, 1};

  write_metadata_file(metadata_fd, &metadata);

  lock_file_by_fd(metadata_fd, 0, 0, F_UNLCK);
}

/**
 * @brief Строит структуру FilePaths
 *
 * Заполняет структуру FilePaths
 *
 * @param base_dir Директория очереди.
 * @param segment Номер сегмента
 * @param paths Структура FilePaths
 */
void build_file_paths(const char *base_dir, uint32_t segment,
                      FilePaths *paths) {
  snprintf(paths->metadata_path, sizeof(paths->metadata_path), "%s/%s",
           base_dir, METADATA_FILE);

  snprintf(paths->segment_path, sizeof(paths->segment_path), "%s/data-%07u.dat",
           base_dir, segment);

  snprintf(paths->index_path, sizeof(paths->index_path), "%s/%s", base_dir,
           INDEX_DATA_FILE);
}

void push_existing_metadata(int metadata_fd, const char *path, char *data,
                            int size) {

  lock_file_by_fd(metadata_fd, 0, 0, F_WRLCK);
  Metadata existMetadata = {};

  read_metadata_file(metadata_fd, &existMetadata);

  printf("currentSegment: %u \n", existMetadata.currentSegment);
  printf("countSegment: %u \n", existMetadata.countSegment);
  printf("currentOffsetWrite: %u \n", existMetadata.currentOffsetWrite);
  printf("countMessage: %u \n", existMetadata.countMessage);

  int is_new_segment = 0;

  if (MAX_SEGMENT_SIZE - (existMetadata.currentOffsetWrite + DATA_HEADER_SIZE) <
      size) {
    is_new_segment = 1;
  }

  char segment_id[32];
  uint32_t segment = existMetadata.countSegment;

  if (is_new_segment) {
    segment = segment + 1;
  }

  FilePaths file_path = {};

  build_file_paths(path, segment, &file_path);

  printf("Путь dataFile: %s \n", file_path.segment_path);

  snprintf(segment_id, sizeof(segment_id), "data-%07u.dat", segment);

  int data_file_fd;

  printf("Новый сегмент: %u \n", is_new_segment);

  if (is_new_segment == 1)
    data_file_fd =
        open(file_path.segment_path, O_CREAT | O_APPEND | O_WRONLY, 0644);
  else
    data_file_fd = open(file_path.segment_path, O_APPEND | O_WRONLY);

  uint8_t time[8];
  writeBufferAtTime(time);

  write_data_file(data_file_fd, data, size, time);

  uint64_t indexOffsetPos = existMetadata.currentOffsetWrite;

  existMetadata.countMessage = existMetadata.countMessage + 1;
  existMetadata.currentOffsetWrite =
      existMetadata.currentOffsetWrite + 16 + size;

  if (is_new_segment) {
    existMetadata.currentSegment = existMetadata.currentSegment + 1;
    existMetadata.countSegment = existMetadata.countSegment + 1;
  }

  int index_file_fd = open(file_path.index_path, O_WRONLY | O_APPEND);

  IndexData index_struct = {existMetadata.countMessage,
                            existMetadata.currentSegment, size, indexOffsetPos,
                            (uint64_t)time};

  write_index_file(index_file_fd, &index_struct);
  write_metadata_file(metadata_fd, &existMetadata);

  lock_file_by_fd(metadata_fd, 0, 0, F_UNLCK);
}

void write_index_file(int index_file_fd, const IndexData *index_struct) {
  uint8_t index_data_buf[28];
  serializeIndexData(index_struct, index_data_buf);
  write(index_file_fd, index_data_buf, sizeof(index_data_buf));
  close(index_file_fd);
}

void write_data_file(int data_file_fd, char *data, int size, uint8_t time[8]) {

  uint32_t size_data = size;
  uint8_t size_data_buffer[4];
  write_u32_be(size_data_buffer, size_data);

  write(data_file_fd, size_data_buffer, sizeof(size_data_buffer));
  write(data_file_fd, time, 8);
  write(data_file_fd, data, sizeof(data));
  close(data_file_fd);
}

void write_metadata_file(int metadata_fd, const Metadata *metadata) {
  uint8_t buffer_metadata[20];
  serializeMetadata(metadata, buffer_metadata);
  lseek(metadata_fd, 0, SEEK_SET);
  write(metadata_fd, buffer_metadata, 20);
}

void read_metadata_file(int metadata_fd, Metadata *metadata) {
  uint8_t buffer[20];
  read(metadata_fd, buffer, 20);
  deserializeMetadata(buffer, metadata);
}
