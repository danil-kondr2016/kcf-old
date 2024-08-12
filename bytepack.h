#pragma once
#ifndef _BYTEPACK_H_
#define _BYTEPACK_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

bool ReadU64LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint64_t *Out);
bool ReadU32LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint32_t *Out);
bool ReadU16LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint16_t *Out);
bool ReadU8(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint8_t *Out);

bool WriteU64LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint64_t In);
bool WriteU32LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint32_t In);
bool WriteU16LE(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint16_t In);
bool WriteU8(uint8_t *Buffer, size_t Size, ptrdiff_t *Offset, uint8_t In);

#endif
