#pragma once
#ifndef _IO_IO_H_
#define _IO_IO_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

typedef struct io_stream_st IO;
typedef struct io_method_st IO_METHOD;

enum {
	IO_CFILE = 1,
	IO_POSIX = 2,
	IO_WIN32 = 3,
};

enum { IO_SEEK_SET, IO_SEEK_CUR, IO_SEEK_END };

IO *IO_create(const IO_METHOD *method);
int IO_close(IO *io);

int IO_read(IO *io, void *buffer, int64_t size, int64_t *n_read);
int IO_write(IO *io, const void *buffer, int64_t size, int64_t *n_write);
int64_t IO_seek(IO *io, int64_t offset, int whence);
int64_t IO_tell(IO *io);

IO *IO_create_fp(FILE *f, int should_close);
IO *IO_open_cfile(const char *path, const char *mode);

IO *IO_create_fd(int fd, int should_close);

// #ifdef _WIN32
// #include <windows.h>
// IO *IO_create_handle(HANDLE hFile, int should_close);
// #else
// IO *IO_create_handle(void *hFile, int should_close);
// #endif

#endif
