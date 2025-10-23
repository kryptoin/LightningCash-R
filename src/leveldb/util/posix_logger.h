// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Logger implementation that can be shared by all environments
// where enough Posix functionality is available.

#ifndef STORAGE_LEVELDB_UTIL_POSIX_LOGGER_H_
#define STORAGE_LEVELDB_UTIL_POSIX_LOGGER_H_

#include "leveldb/env.h"
#include <algorithm>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

namespace leveldb {
class PosixLogger : public Logger {
private:
  FILE *file_;
  uint64_t (*gettid_)();

public:
  PosixLogger(FILE *f, uint64_t (*gettid)()) : file_(f), gettid_(gettid) {}
  virtual ~PosixLogger() { fclose(file_); }
  virtual void Logv(const char *format, va_list ap) {
    const uint64_t thread_id = (*gettid_)();

    char buffer[500];
    for (int iter = 0; iter < 2; iter++) {
      char *base;
      int bufsize;
      if (iter == 0) {
        bufsize = sizeof(buffer);
        base = buffer;
      } else {
        bufsize = 30000;
        base = new char[bufsize];
      }
      char *p = base;
      char *limit = base + bufsize;

      struct timeval now_tv;
      gettimeofday(&now_tv, NULL);
      const time_t seconds = now_tv.tv_sec;
      struct tm t;
      localtime_r(&seconds, &t);
      p += snprintf(p, limit - p, "%04d/%02d/%02d-%02d:%02d:%02d.%06d %llx ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour,
                    t.tm_min, t.tm_sec, static_cast<int>(now_tv.tv_usec),
                    static_cast<long long unsigned int>(thread_id));

      if (p < limit) {
        va_list backup_ap;
        va_copy(backup_ap, ap);
        p += vsnprintf(p, limit - p, format, backup_ap);
        va_end(backup_ap);
      }

      if (p >= limit) {
        if (iter == 0) {
          continue;

        } else {
          p = limit - 1;
        }
      }

      if (p == base || p[-1] != '\n') {
        *p++ = '\n';
      }

      assert(p <= limit);
      fwrite(base, 1, p - base, file_);
      fflush(file_);
      if (base != buffer) {
        delete[] base;
      }
      break;
    }
  }
};

} // namespace leveldb

#endif
