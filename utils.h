#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdint.h>

int kcf_fskip(FILE *File, size_t SizeToSkip);

uint16_t read_u16le(uint8_t *buf);
uint32_t read_u32le(uint8_t *buf);
uint64_t read_u64le(uint8_t *buf);

#endif
