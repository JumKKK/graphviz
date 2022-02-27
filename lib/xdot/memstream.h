#pragma once

#ifdef _MSC_VER
#define INTERNAL __declspec(dllimport)
#elif defined(__GNUC__)
#define INTERNAL __attribute__((visibility("internal")))
#else
#define INTERNAL /* nothing */
#endif

#include <stddef.h>
#include <stdio.h>

/// in-memory file
typedef struct {

  /// 
  FILE *file;
  char *buffer;
  size_t buffer_size;

  char *path_;
} memstream_t;

INTERNAL int memstream_new(memstream_t *m);

INTERNAL int memstream_sync(memstream_t *m);

INTERNAL void memstream_free(memstream_t *m);

#undef INTERNAL
