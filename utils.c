#include "data.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int is_directory_exist(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0) {
    return S_ISDIR(st.st_mode);
  }
  return 0;
}

void write_u32_be(uint8_t *buf, uint32_t value) {
  buf[0] = (value >> 24) & 0xFF;
  buf[1] = (value >> 16) & 0xFF;
  buf[2] = (value >> 8) & 0xFF;
  buf[3] = value & 0xFF;
}

uint32_t read_u32_be(const uint8_t *buf) {
  return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
         ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

void write_u64_be(uint8_t *buf, uint64_t value) {
  for (int i = 0; i < 8; i++) {
    buf[i] = (value >> (56 - 8 * i)) & 0xFF;
  }
}

uint64_t read_u64_be(const uint8_t *buf) {
  uint64_t val = 0;
  for (int i = 0; i < 8; i++) {
    val |= ((uint64_t)buf[i]) << (56 - 8 * i);
  }
  return val;
}

void serializeMetadata(const Metadata *meta, uint8_t *buffer) {
  write_u32_be(buffer, meta->currentSegment);
  write_u64_be(buffer + 4, meta->currentOffsetWrite);
  write_u32_be(buffer + 12, meta->countSegment);
  write_u32_be(buffer + 16, meta->countMessage);
}

void deserializeMetadata(const uint8_t *buffer, Metadata *meta) {
  meta->currentSegment = read_u32_be(buffer);
  meta->currentOffsetWrite = read_u64_be(buffer + 4);
  meta->countSegment = read_u32_be(buffer + 12);
  meta->countMessage = read_u32_be(buffer + 16);
}
