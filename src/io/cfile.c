#define _CRT_SECURE_NO_WARNINGS
#include <io/io.h>

#include "io_local.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

static int _cfile_read(IO *io, void *buffer, int64_t size, int64_t *n_read);
static int _cfile_write(IO *io, const void *buffer, int64_t size,
                        int64_t *n_write);
static int64_t _cfile_seek(IO *io, int64_t offset, int whence);
static int64_t _cfile_tell(IO *io);
static int _cfile_close(IO *io);

static const IO_METHOD _cfile_method = {
    IO_CFILE, _cfile_read, _cfile_write, _cfile_seek, _cfile_tell, _cfile_close,
};

#define CHUNK_SIZE 1073741824L

static int _cfile_read(IO *io, void *buffer, int64_t size, int64_t *n_read)
{
	FILE *file;
	size_t to_read;
	int64_t read_length;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	if (sizeof(size_t) == 4 && size > INT32_MAX) {
		read_length = 0;
		do {
			size_t read_4 = 0;

			to_read = CHUNK_SIZE;
			if (size < to_read)
				to_read = size;
			read_4 = fread(buffer, 1, to_read, file);
			if (ferror(file))
				return -1;
			read_length += read_4;
		} while (size > 0);
	} else {
		to_read     = size;
		read_length = fread(buffer, 1, to_read, file);
	}

	if (n_read)
		*n_read = read_length;
	return 0;
}

static int _cfile_write(IO *io, const void *buffer, int64_t size,
                        int64_t *n_write)
{
	FILE *file;
	size_t to_write;
	int64_t write_length;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	if (sizeof(size_t) == 4 && size > INT32_MAX) {
		write_length = 0;
		do {
			size_t write_4 = 0;

			to_write = CHUNK_SIZE;
			if (size < to_write)
				to_write = size;
			write_4 = fwrite(buffer, 1, to_write, file);
			if (ferror(file))
				return -1;
			write_length += write_4;
		} while (size > 0);
	} else {
		to_write     = size;
		write_length = fwrite(buffer, 1, to_write, file);
	}

	if (n_write)
		*n_write = write_length;
	return 0;
}

static int64_t _cfile_seek(IO *io, int64_t offset, int whence)
{
	FILE *file;
	int64_t ret = 0;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	if (sizeof(long) == 4) {
#ifdef _WIN32
		ret = _fseeki64(file, offset, whence);
#else
		ret = fseeko64(file, offset, whence);
#endif
	} else {
		ret = fseek(file, offset, whence);
	}

	return ret;
}

static int64_t _cfile_tell(IO *io)
{
	FILE *file;
	int64_t ret = 0;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	if (sizeof(long) == 4) {
#ifdef _WIN32
		ret = _ftelli64(file);
#else
		ret = ftello64(file);
#endif
	} else {
		ret = ftell(file);
	}

	return ret;
}

static int _cfile_close(IO *io)
{
	FILE *file;
	int ret;

	assert(io);
	if ((io->flags & IO_FLAG_CLOSE) == 0)
		return 0;
	if (!io->ptr)
		return 0;

	file = (FILE *)io->ptr;

	ret = fclose(file);
	if (ret == EOF)
		ret = -1;

	io->ptr = NULL;
	return ret;
}

IO *IO_create_fp(FILE *f, int should_close)
{
	IO *result;

	(void)should_close;

	result = IO_create(&_cfile_method);
	if (!result)
		return result;

	result->ptr = (void *)f;
	if (should_close)
		result->flags |= IO_FLAG_CLOSE;

	return result;
}

IO *IO_create_cfile(const char *path, const char *mode)
{
	IO *result;
	FILE *f;

	f = fopen(path, mode);
	if (!f)
		return NULL;

	result = IO_create_fp(f, 1);
	if (!result)
		return NULL;

	return result;
}
