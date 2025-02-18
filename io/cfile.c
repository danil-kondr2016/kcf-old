#define _CRT_SECURE_NO_WARNINGS
#include <io/io.h>

#include "io_local.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

static int64_t _cfile_read(IO *io, void *buffer, int64_t size);
static int64_t _cfile_write(IO *io, const void *buffer, int64_t size);
static int64_t _cfile_seek(IO *io, int64_t offset, int whence);
static int64_t _cfile_tell(IO *io);
static int _cfile_flush(IO *io);
static int _cfile_close(IO *io);

static const IO_METHOD _cfile_method = {
    IO_CFILE, 
    _cfile_read, 
    _cfile_write, 
    _cfile_seek,
    _cfile_tell, 
    _cfile_flush,
    _cfile_close,
};

#define CHUNK_SIZE 1073741824L

static int64_t _cfile_read(IO *io, void *buffer, int64_t size)
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

			size -= read_4;
			read_length += read_4;
			buffer += read_4;
		} while (size > 0);
	} else {
		to_read     = size;
		read_length = fread(buffer, 1, to_read, file);
	}

	return read_length;
}

static int64_t _cfile_write(IO *io, const void *buffer, int64_t size)
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
			size -= write_4;
			buffer += write_4;
		} while (size > 0);
	} else {
		to_write     = size;
		write_length = fwrite(buffer, 1, to_write, file);
	}

	return write_length;
}

#ifdef _WIN32
#define _cfseek _fseeki64
#define _cftell _ftelli64
#else
#define _cfseek fseeko
#define _cftell ftello
#endif

static int64_t _cfile_seek(IO *io, int64_t offset, int whence)
{
	FILE *file;
	int64_t ret = 0;
	int std_whence;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	switch (whence) {
	case IO_SEEK_CUR: std_whence = SEEK_CUR; break;
	case IO_SEEK_SET: std_whence = SEEK_SET; break;
	case IO_SEEK_END: std_whence = SEEK_END; break;
	default:
		return -1;
	}

	ret = _cfseek(file, offset, std_whence);

	return ret;
}

static int64_t _cfile_tell(IO *io)
{
	FILE *file;
	int64_t ret = 0;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	ret = _cftell(file);

	return ret;
}

static int _cfile_flush(IO *io)
{
	FILE *file;
	int ret = 0;

	assert(io);
	assert(io->ptr);
	file = (FILE *)io->ptr;

	ret = fflush(file);
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
