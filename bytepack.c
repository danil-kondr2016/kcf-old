#include "bytepack.h"

#include <assert.h>

static
inline
bool _read_u8(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint8_t *Out)
{
	uint8_t result;

	if (*Offset >= Size)
		return false;
	*Out = Buffer[*Offset];
	(*Offset)++;

	return true;
}


static
inline
bool _read_u16(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint16_t *Out)
{
	uint8_t low, high;
	bool low_read, high_read;

	low_read = _read_u8(Buffer, Size, Offset, &low);
	high_read = _read_u8(Buffer, Size, Offset, &high);

	*Out = low | ((uint16_t)high << 8);
	return low_read && high_read;
}


static
inline
bool _read_u32(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint32_t *Out)
{
	uint16_t low, high;
	bool low_read, high_read;

	low_read = _read_u16(Buffer, Size, Offset, &low);
	high_read = _read_u16(Buffer, Size, Offset, &high);

	*Out = low | ((uint32_t)high << 16);
	return low_read && high_read;
}


static
inline
bool _read_u64(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint64_t *Out)
{
	uint32_t low, high;
	bool low_read, high_read;

	low_read = _read_u32(Buffer, Size, Offset, &low);
	high_read = _read_u32(Buffer, Size, Offset, &high);

	*Out = low | ((uint64_t)high << 32);
	return low_read && high_read;
}

bool ReadU64LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint64_t *Out)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);
	assert(Out);

	if (!Offset)
		Offset = &offset_if_0;

	return _read_u64(Buffer, Size, Offset, Out);
}

bool ReadU32LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint32_t *Out)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);
	assert(Out);

	if (!Offset)
		Offset = &offset_if_0;

	return _read_u32(Buffer, Size, Offset, Out);
}

bool ReadU16LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint16_t *Out)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);
	assert(Out);

	if (!Offset)
		Offset = &offset_if_0;

	return _read_u16(Buffer, Size, Offset, Out);
}

bool ReadU8(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint8_t *Out)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);
	assert(Out);

	if (!Offset)
		Offset = &offset_if_0;

	return _read_u8(Buffer, Size, Offset, Out);
}

static
inline
bool _write_u8(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint8_t In)
{
	if (*Offset >= Size)
		return false;
	Buffer[*Offset] = In;
	(*Offset)++;
	return true;
}

static
inline
bool _write_u16(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint16_t In)
{
	return _write_u8(Buffer, Size, Offset, In&0xFF)
		&& _write_u8(Buffer, Size, Offset, (In>>8)&0xFF);
}

static
inline
bool _write_u32(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint32_t In)
{
	return _write_u16(Buffer, Size, Offset, In&0xFFFF)
		&& _write_u16(Buffer, Size, Offset, (In>>16)&0xFFFF);
}


static
inline
bool _write_u64(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint64_t In)
{
	return _write_u32(Buffer, Size, Offset, In&0xFFFFFFFFL)
		&& _write_u32(Buffer, Size, Offset, (In>>32)&0xFFFFFFFFL);
}


bool WriteU64LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint64_t In)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);

	if (!Offset)
		Offset = &offset_if_0;

	return _write_u64(Buffer, Size, Offset, In);
}

bool WriteU32LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint32_t In)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);

	if (!Offset)
		Offset = &offset_if_0;

	return _write_u32(Buffer, Size, Offset, In);

}

bool WriteU16LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint16_t In)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);

	if (!Offset)
		Offset = &offset_if_0;

	return _write_u16(Buffer, Size, Offset, In);

}

bool WriteU8(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint8_t In)
{
	ptrdiff_t offset_if_0 = 0;
	assert(Buffer);

	if (!Offset)
		Offset = &offset_if_0;

	return _write_u8(Buffer, Size, Offset, In);
}
