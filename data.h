#ifndef DATA_H
#define DATA_H

#include "message.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int lock_file_by_fd(int fd, int start, int len, short lock_type);

void push(const char *file_name, char *data, int size);

void pop(const char *queue_name, unsigned char *tailer_name);

void getStatsQueue(const char *queue_name);

void checkNewMessage(const char *queue_name);

void getMessage(const char *queue_name, char *tailer_name);

void push_new_metadata(int metadata_fd, const char *path, char *data, int size);

void push_existing_metadata(int metadata_fd, const char *path, char *data,
                            int size);

#pragma pack(push, 1)
typedef struct {
  uint32_t size;
  uint64_t time;

} MessageData;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint32_t currentSegment;
  uint64_t currentOffsetWrite;
  uint32_t countSegment;
  uint32_t countMessage;
} Metadata;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint32_t messageId;
  uint32_t segmentId;
  uint32_t size;
  uint64_t offsetInData;
  uint64_t time;
} IndexData;
#pragma pack(pop)

typedef struct {
  char metadata_path[512];
  char segment_path[512];
  char index_path[512];
} FilePaths;

void build_file_paths(const char *base_dir, uint32_t segment, FilePaths *paths);

void write_index_file(int index_file_fd, const IndexData *index_struct);

void write_data_file(int data_file_fd, char *data, int size, uint8_t time[8]);

void write_metadata_file(int metadata_fd, const Metadata *metadata);

void read_metadata_file(int metadata_fd, Metadata *metadata);

#endif
