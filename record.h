#pragma once
#ifndef _RECORD_H_
#define _RECORD_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "kcf.h"
#include "errors.h"

#define KCF_HAS_ADDED_SIZE_4 0x80
#define KCF_HAS_ADDED_SIZE_8 0xC0

struct KcfRecordHeader
{
	uint16_t HeadCRC;
	uint8_t  HeadType;
	uint8_t  HeadFlags;
	uint16_t HeadSize;

	uint64_t AddedSize;
};

struct KcfFileHeader
{
	char     *FileName;
	uint64_t Timestamp;
	uint32_t FileCRC32;
	uint32_t FileType;
	uint32_t CompressionInfo;
	uint16_t FileNameSize;
	uint8_t FileFlags;
};

KCFERROR ScanArchiveForMarker(HKCF hKCF);

#endif
