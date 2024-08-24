#define _CRT_SECURE_NO_WARNINGS

#include <io/io.h>

#include "io_local.h"
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <assert.h>
#include <fcntl.h>

static int _fd_read(IO *io, void *buffer, int64_t size, int64_t *n_read);
static int _fd_write(IO *io, const void *buffer, int64_t size, int64_t *n_write);
static int64_t _fd_seek(IO *io, int64_t offset, int whence);
static int64_t _fd_tell(IO *io);
static int _fd_close(IO *io);

static const IO_METHOD _fd_method = {
	IO_POSIX,
	_fd_read,
	_fd_write,
	_fd_seek,
	_fd_tell,
	_fd_close
};

#define CHUNK_SIZE 1073741824L

#ifdef _WIN32
#define _fd_read_ret int
#define _fd_read_count int
#define _fd_read_fn _read
#define _fd_read_chunked(x) ((x) > INT32_MAX)
#else
#define _fd_read_ret ssize_t
#define _fd_read_count size_t
#define _fd_read_fn read
#define _fd_read_chunked(x) ((sizeof(size_t) == 4) && ((x) > INT32_MAX))
#endif

static int _fd_read(IO *io, void *buffer, int64_t size, int64_t *n_read)
{
	_fd_read_ret ret;
	_fd_read_count to_read;
	int64_t read_length = 0;
	
	assert(io);
	if (_fd_read_chunked(size)) {
		do {
			to_read = CHUNK_SIZE;
			if (size < to_read)
				to_read = size;

			ret = _fd_read_fn(io->handle, buffer, to_read);
			if (ret < 0)
				break;
		
			size -= ret;
			read_length += ret;
		} while (size > 0);
	} else {
		ret = _fd_read_fn(io->handle, buffer, size);
		if (ret >= 0)
			read_length = ret;
		else
			read_length = 0;
	}
	
	if (n_read)
		*n_read = read_length;
	return ret >= 0 ? 0 : -1;
}

#undef _fd_read_ret
#undef _fd_read_count
#undef _fd_read_fn
#undef _fd_read_chunked

#ifdef _WIN32
#define _fd_write_ret int
#define _fd_write_count int
#define _fd_write_fn _write
#define _fd_write_chunked(x) ((x) > INT32_MAX)
#else
#define _fd_write_ret ssize_t
#define _fd_write_count size_t
#define _fd_write_fn write
#define _fd_write_chunked(x) ((sizeof(size_t) == 4) && ((x) > INT32_MAX))
#endif

static int _fd_write(IO *io, const void *buffer, int64_t size, int64_t *n_write)
{
	_fd_write_ret ret;
	_fd_write_count to_write;
	int64_t write_length = 0;
	
	assert(io);
	if (_fd_write_chunked(size)) {
		do {
			to_write = CHUNK_SIZE;
			if (size < to_write)
				to_write = size;

			ret = _fd_write_fn(io->handle, buffer, to_write);
			if (ret < 0)
				break;
		
			size -= ret;
			write_length += ret;
		} while (size > 0);
	} else {
		ret = _write(io->handle, buffer, size);
		if (ret >= 0)
			write_length = ret;
		else
			write_length = 0;
	}
	
	if (n_write)
		*n_write = write_length;
	return ret >= 0 ? 0 : -1;
}

#ifdef _WIN32
#define _fd_seek_ret int64_t
#define _fd_seek_fn  _lseeki64
#else
#define _fd_seek_ret off64_t
#define _fd_seek_fn  lseeko64
#endif

static int64_t _fd_seek(IO *io, int64_t offset, int whence)
{
	_fd_seek_ret ret;

	assert(io);
	switch (whence) {
	case IO_SEEK_SET:
		ret = _fd_seek_fn(io->handle, offset, SEEK_SET);
		break;
	case IO_SEEK_CUR:
		ret = _fd_seek_fn(io->handle, offset, SEEK_CUR);
		break;
	case IO_SEEK_END:
		ret = _fd_seek_fn(io->handle, offset, SEEK_END);
		break;
	}

	return ret;
}

static int64_t _fd_tell(IO *io)
{
	_fd_seek_ret ret;

	assert(io);
	ret = _fd_seek_fn(io->handle, 0, SEEK_CUR);
	return ret;
}

static int _fd_close(IO *io)
{
	int ret;
	
	assert(io);
	
	if ((io->flags & IO_FLAG_CLOSE) == 0)
		return 0;

#ifdef _WIN32
	ret = _close(io->handle);
#else
	ret = close(io->handle);
#endif
	if (ret == 0)
		io->handle = (uintptr_t)-1;
	return ret;
}

IO *IO_create_fd(int fd, int should_close)
{
	IO *result;

	result = IO_create(&_fd_method);
	if (!result)
		return NULL;

	result->handle = fd;
	if (should_close)
		result->flags |= IO_FLAG_CLOSE;

	return result;
}
