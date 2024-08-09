#include "utils.h"

uint16_t read_u16le(uint8_t *buf)
{
	return buf[0] | (buf[1]<<8);
}

uint32_t read_u32le(uint8_t *buf)
{
	return (uint32_t)read_u16le(buf) | ((uint32_t)read_u16le(buf+2) << 16);
}

uint64_t read_u64le(uint8_t *buf) 
{
	return (uint64_t)read_u32le(buf)
		| ((uint64_t)read_u32le(buf+4) << 32);
}

