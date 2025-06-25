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

// блокировка конца файла
int lock_file_by_fd(int fd, int start, int len, short lock_type) {
  struct flock fl = {
      .l_type = lock_type,  // write‑lock (эксклюзивная)
      .l_whence = SEEK_SET, // от начала файла
      .l_start = 0,         // смещение
      .l_len = len          // 0 = «до конца файла» → весь файл
  };

  /* F_SETLKW = «ждать, пока не получится поставить блокировку» */
  return fcntl(fd, F_SETLKW, &fl);
}
