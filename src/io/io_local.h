#pragma once
#ifndef _IO_LOCAL_H_
#define _IO_LOCAL_H_

#include <io/io.h>

enum {
	IO_FLAG_READ  = (1 << 0),
	IO_FLAG_WRITE = (1 << 1),
	IO_FLAG_SEEK  = (1 << 2),
	IO_FLAG_CLOSE = (1 << 3),
};

struct io_stream_st {
	const IO_METHOD *method;
	union {
		void *ptr;
		uintptr_t handle;
	};
	unsigned flags;
};

struct io_method_st {
	uintptr_t type;

	int64_t (*read)(IO *io, void *buffer, int64_t size);
	int64_t (*write)(IO *io, const void *buffer, int64_t size);
	int64_t (*seek)(IO *io, int64_t offset, int whence);
	int64_t (*tell)(IO *io);
	int (*close)(IO *io);
};

#endif
