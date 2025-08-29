#ifndef DATA_H
#define DATA_H

#include "message.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>

int lock_file_by_fd(int fd, int start, int len, short lock_type);

void push(const char *file_name, const unsigned char *data, int size);

void pop(const char *file_name, unsigned char *data);

void getStatsQueue();

void checkNewMessage();

void push_new_metadata(int metadata_fd, const char *path,
                       const unsigned char *data, int size);

void push_existing_metadata(int metadata_fd, const char *path,
                            const unsigned char *data, int size);

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
  uint32_t segmentId;
  uint32_t size;
  uint64_t offsetInData;
} IndexData;
#pragma pack(pop)

typedef struct {
  char metadata_path[512];
  char segment_path[512];
  char index_path[512];
} FilePaths;

void build_file_paths(const char *base_dir, uint32_t segment, FilePaths *paths);

#endif
