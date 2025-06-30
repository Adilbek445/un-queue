#include "data.h"

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
