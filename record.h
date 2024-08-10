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

#endif
