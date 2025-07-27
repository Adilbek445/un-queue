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

typedef struct {
  uint32_t segmentId;
  uint64_t offsetInData;
} IndexData;

#endif
