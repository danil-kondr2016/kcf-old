#include "io_local.h"
#include <io/io.h>

#include <stdlib.h>

IO *IO_create(const IO_METHOD *method)
{
	IO *result;

	if (!method)
		return NULL;

	result = calloc(1, sizeof(struct io_stream_st));
	if (!result)
		return NULL;

	result->method = method;
	return result;
}

int IO_close(IO *io)
{
	int ret;

	if (io->method->close)
		ret = io->method->close(io);
	else
		ret = 0;

	free(io);
	return ret;
}

int64_t IO_read(IO *io, void *buffer, int64_t size)
{
	int64_t ret;

	if (!io)
		return -1;

	if (io->method->read)
		ret = io->method->read(io, buffer, size);
	else
		ret = -2;

	return ret;
}

int64_t IO_write(IO *io, const void *buffer, int64_t size)
{
	int64_t ret;

	if (!io)
		return -1;

	if (io->method->write)
		ret = io->method->write(io, buffer, size);
	else
		ret = -2;

	return ret;
}

int64_t IO_seek(IO *io, int64_t offset, int whence)
{
	int64_t ret;

	if (!io)
		return -1;

	if (io->method->seek)
		ret = io->method->seek(io, offset, whence);
	else
		ret = -2;

	return ret;
}

int64_t IO_tell(IO *io)
{
	int64_t ret;

	if (!io)
		return -1;

	if (io->method->tell)
		ret = io->method->tell(io);
	else
		ret = -2;

	return ret;
}
