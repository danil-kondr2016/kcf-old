#include "io_local.h"
#include <io/io.h>

#include <assert.h>

#ifdef _WIN32

static int64_t _w32_read(IO *io, void *buffer, int64_t size);
static int64_t _w32_write(IO *io, const void *buffer, int64_t size);
static int64_t _w32_seek(IO *io, int64_t offset, int whence);
static int64_t _w32_tell(IO *io);
static int _w32_flush(IO *io);
static int _w32_close(IO *io);

static const IO_METHOD _w32_method = {
	IO_WIN32,
	_w32_read,
	_w32_write,
	_w32_seek,
	_w32_tell,
	_w32_flush,
	_w32_close,
};

#define CHUNK_SIZE 1073741824L

static int64_t _w32_read(IO *io, void *buffer, int64_t size)
{
	HANDLE hFile;
	int64_t read_length = 0;
	int64_t remaining, to_read;
	DWORD n_read;
	BOOL result;

	assert(io);
	hFile = (HANDLE)io->handle;
	assert(hFile != INVALID_HANDLE_VALUE);

	if (size > INT32_MAX) {
		do {
			to_read = CHUNK_SIZE;
			if (size < to_read)
				to_read = size;

			result = ReadFile(hFile, buffer, to_read, &n_read, NULL);
			if (!result)
				return -1;
			if (n_read == 0)
				break;

			size -= n_read;
			read_length += n_read;
			buffer += n_read;
		} while (size > 0);
	} else {
		result = ReadFile(hFile, buffer, size, &n_read, NULL);
		if (!result)
			return -1;
		read_length = n_read;
	}

	return read_length;
}

static int64_t _w32_write(IO *io, const void *buffer, int64_t size)
{

	HANDLE hFile;
	int64_t write_length = 0;
	int64_t remaining, to_write;
	DWORD n_write;
	BOOL result;

	assert(io);
	hFile = (HANDLE)io->handle;
	assert(hFile != INVALID_HANDLE_VALUE);

	if (size > INT32_MAX) {
		do {
			to_write = CHUNK_SIZE;
			if (size < to_write)
				to_write = size;

			result = WriteFile(hFile, buffer, to_write, &n_write, NULL);
			if (!result)
				return -1;
			if (n_write == 0)
				break;

			size -= n_write;
			write_length += n_write;
			buffer += n_write;
		} while (size > 0);
	} else {
		result = WriteFile(hFile, buffer, size, &n_write, NULL);
		if (!result)
			return -1;
		write_length = n_write;
	}

	return write_length;
}

static int64_t _w32_seek(IO *io, int64_t offset, int whence)
{
	LONG offset_low, offset_high;
	LONG new_offset_low;
	int64_t new_offset;
	HANDLE hFile;

	assert(io);
	hFile = (HANDLE)io->handle;

	offset_low = offset & 0xFFFFFFFFUL;
	offset_high = (offset >> 32) & 0xFFFFFFFFUL;

	switch (whence) {
	case IO_SEEK_SET:
		new_offset_low = SetFilePointer(hFile, offset_low, &offset_high, FILE_BEGIN);
		break;		
	case IO_SEEK_CUR:
		new_offset_low = SetFilePointer(hFile, offset_low, &offset_high, FILE_CURRENT);
		break;
	case IO_SEEK_END:
		new_offset_low = SetFilePointer(hFile, offset_low, &offset_high, FILE_END);
		break;
	}
	
	if (new_offset_low == INVALID_SET_FILE_POINTER) {
		if (GetLastError() != NO_ERROR)
			return -1;
	}

	if (offset_high == 0) {
		new_offset = new_offset_low;
	} else {
		new_offset = (uint32_t)new_offset_low | ((uint64_t)(offset_high) << 32);
	}
	
	return new_offset;
}

static int64_t _w32_tell(IO *io)
{
	return _w32_seek(io, 0, IO_SEEK_CUR);
}

static int _w32_flush(IO *io)
{
	return 0;
}

static int _w32_close(IO *io)
{
	HANDLE hFile;

	assert(io);
	if (io->flags & IO_FLAG_CLOSE) {
		hFile = (HANDLE)io->handle;

		if (!CloseHandle(hFile)) {
			return -1;
		}
	
		io->handle = 0;
	}

	return 0;
}

IO *IO_create_handle(HANDLE hFile, int should_close)
{
	IO *result;

	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	result = IO_create(&_w32_method);
	if (!result)
		return NULL;

	result->handle = (uintptr_t)hFile;
	if (should_close)
		result->flags |= IO_FLAG_CLOSE;

	return result;
}

#else

IO *IO_create_handle(void *hFile, int should_close)
{
	return NULL;
}

#endif
