#ifndef UTILS_H
#define UTILS_H

#include "data.h"
#include "message.h"
#include <stdint.h>
int is_directory_exist(const char *path);

void write_u32_be(uint8_t *buf, uint32_t value);
uint32_t read_u32_be(const uint8_t *buf);

void write_u64_be(uint8_t *buf, uint64_t value);
uint64_t read_u64_be(const uint8_t *buf);

void serializeMetadata(const Metadata *meta, uint8_t *buffer);
void deserializeMetadata(const uint8_t *buffer, Metadata *meta);

void serializeIndexData(const IndexData *entry, uint8_t *buffer);
void deserializeIndexData(const uint8_t *buffer, IndexData *entry);

void writeBufferAtTime(uint8_t *buffer);

#endif
