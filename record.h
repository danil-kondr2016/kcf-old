#pragma once
#ifndef _RECORD_H_
#define _RECORD_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "kcf.h"
#include "errors.h"

#define KCF_HAS_ADDED_DATA_CRC32 0x20
#define KCF_HAS_ADDED_SIZE_4 0x80
#define KCF_HAS_ADDED_SIZE_8 0xC0

enum KcfRecordType
{
	KCF_MARKER         = '!',
	KCF_ARCHIVE_HEADER = 'A',
	KCF_FILE_HEADER    = 'F',
	KCF_DATA_FRAGMENT  = 'D',
};

struct KcfRecordHeader
{
	uint16_t HeadCRC;
	uint8_t  HeadType;
	uint8_t  HeadFlags;
	uint16_t HeadSize;

	union {
		struct {
		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			uint32_t AddedSizeLow;
			uint32_t AddedSizeHigh;
		#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			uint32_t AddedSizeHigh;
			uint32_t AddedSizeLow;
		#else
		#error "Unrecognized byte order"
		#endif
		};
		uint64_t AddedSize;
	};
	uint32_t AddedDataCRC32;
};

#define KCF_FILE_HAS_TIMESTAMP  0x01
#define KCF_FILE_HAS_FILE_CRC32 0x02
#define KCF_FILE_HAS_UNPACKED_4 0x04
#define KCF_FILE_HAS_UNPACKED_8 0x0C

#define KCF_DIRECTORY    'd'
#define KCF_REGULAR_FILE 'F'

struct KcfFileHeader
{
	char     *FileName;
	uint64_t TimeStamp;
	uint64_t UnpackedSize;
	uint32_t FileCRC32;
	uint32_t CompressionInfo;
	uint16_t FileNameSize;
	uint8_t  FileFlags;
	uint8_t  FileType;
};

struct KcfRecord
{
	struct KcfRecordHeader Header;
	uint8_t *Data;
	size_t   DataSize;
};

KCFERROR ScanArchiveForMarker(HKCF hKCF);
KCFERROR WriteArchiveMarker(HKCF hKCF);

KCFERROR ReadRecord(HKCF hKCF, struct KcfRecord *Record);
KCFERROR SkipRecord(HKCF hKCF);

bool ValidateRecord(struct KcfRecord *Record);
void ClearRecord(struct KcfRecord *Record);

bool IsAddedDataAvailable(HKCF hKCF);
KCFERROR ReadAddedData(HKCF hKCF, void *Destination, size_t BufferSize,
		size_t *BytesRead);

struct KcfArchiveHeader
{
	uint16_t ArchiveVersion;
};

KCFERROR RecordToArchiveHeader(struct KcfRecord *, struct KcfArchiveHeader *);
KCFERROR RecordToFileHeader(struct KcfRecord *, struct KcfFileHeader *);

void ClearFileHeader(struct KcfFileHeader *);

#endif
