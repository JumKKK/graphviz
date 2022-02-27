#include <assert.h>
#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <xdot/memstream.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include <errhandlingapi.h>
#include <fileapi.h>
#endif

int memstream_new(memstream_t *m) {
  assert(m != NULL);

  memset(m, 0, sizeof(*m));

#ifdef HAVE_OPEN_MEMSTREAM
  m->file = open_memstream(&m->buffer, &m->buffer_size);
  if (m->file == NULL)
    return errno;

  // if we do not have open_memstream, fall back to using temporary on-disk file
#elif defined(_MSC_VER)

  char tmp[MAX_PATH + 1];
  DWORD len = GetTempPath2A(sizeof(tmp), tmp);
  if (len == 0)
    return GetLastError();
  assert(len < sizeof(tmp));

  char *path = malloc(strlen(tmp) + strlen("tmp.XXXXXX") + 1);
  if (path == NULL)
    return ENOMEM;

  if (mktemp(path) == NULL) {
    free(path);
    return errno;
  }

  FILE *file = fopen(path, "wb+");
  if (file == NULL) {
    free(path);
    return errno;
  }

  m->path_ = path;
  m->file = file;

#else

  char *tmp = getenv("TMPDIR");
  if (tmp == NULL)
    tmp = "/tmp";

  char *path = malloc(strlen(tmp) + 1 + strlen("tmp.XXXXXX") + 1);
  if (path == NULL)
    return ENOMEM;

  int fd = mkstemp(path);
  if (fd < 0) {
    free(path);
    return errno;
  }

  FILE *file = fdopen(fd, "rw");
  if (file == NULL) {
    int err = errno;
    (void)close(fd);
    free(path);
    return err;
  }

  m->path_ = path;
  m->file = file;

#endif

  return 0;
}

static void discard(char **buffer) {
  assert(buffer != NULL);
  free(*buffer);
  *buffer = NULL;
}

int memstream_sync(memstream_t *m) {
  assert(m != NULL);

  if (fflush(m->file) != 0)
    return errno;

#ifndef HAVE_OPEN_MEMSTREAM

  discard(&m->buffer);

  // determine the current size of the temporary file
  struct stat st;
  if (stat(m->path_, &st) < 0)
    return errno;

  // allocate new space for this content
  m->buffer = malloc(st.st_size + 1);
  if (m->buffer == NULL)
    return ENOMEM;

  // read the file content into this buffer
  if (fseek(m->file, 0, SEEK_SET) < 0)
    return errno;
  if (fread(m->buffer, 1, st.st_size, m->file) != st.st_size) {
    discard(&m->buffer);
    return errno;
  }
  m->buffer[st.st_size] = '\0';
  m->buffer_size = st.st_size;

  // return the handle to the end of the file for further appending
  if (fseek(m->file, 0, SEEK_END) < 0)
    return errno;
  
#endif

  return 0;
}

void memstream_free(memstream_t *m) {

  // alloc harmless memstream_free of NULL
  if (m == NULL)
    return;

  (void)fclose(m->file);
  discard(&m->buffer);

  memset(m, 0, sizeof(*m));
}
