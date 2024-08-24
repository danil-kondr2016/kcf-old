/**
 * \file record.h
 *
 * Functions for working with records inisde KCF archive.
 */

#pragma once
#ifndef _RECORD_H_
#define _RECORD_H_

#include <stdbool.h>
#include <stdint.h>

#include <kcf/errors.h>

#define KCF_HAS_ADDED_DATA_CRC32 0x20
#define KCF_HAS_ADDED_SIZE_4     0x80
#define KCF_HAS_ADDED_SIZE_8     0xC0

enum KcfRecordType {
	KCF_MARKER         = '!',
	KCF_ARCHIVE_HEADER = 'A',
	KCF_FILE_HEADER    = 'F',
	KCF_DATA_FRAGMENT  = 'D',
};

struct KcfRecord {
	uint16_t HeadCRC;
	uint8_t HeadType;
	uint8_t HeadFlags;
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

	uint8_t *Data;
	size_t DataSize;
};

struct KcfArchiveHeader {
	uint16_t ArchiveVersion;
};

/* Record manipulation functions */

uint16_t rec_calculate_CRC(struct KcfRecord *Record);
bool rec_validate(struct KcfRecord *Record);
void rec_fix(struct KcfRecord *Record);
void rec_clear(struct KcfRecord *Record);
KCFERROR rec_copy(struct KcfRecord *Destination, struct KcfRecord *Source);

bool rec_to_buffer(struct KcfRecord *Record, uint8_t *Buffer, size_t Size);

KCFERROR rec_to_archive_header(struct KcfRecord *, struct KcfArchiveHeader *);
KCFERROR rec_from_archive_header(struct KcfArchiveHeader *, struct KcfRecord *);
void ahdr_clear(struct KcfArchiveHeader *);

/* Flag probing functions */

static inline bool rec_has_added_size(struct KcfRecord *record)
{
	if (!record)
		return false;

	if (record->HeadFlags & KCF_HAS_ADDED_SIZE_4)
		return true;

	return false;
}

static inline bool rec_has_added_data_CRC(struct KcfRecord *record)
{
	if (!record)
		return false;

	return !!(record->HeadFlags & KCF_HAS_ADDED_DATA_CRC32);
}

bool rec_has_added_size_8(struct KcfRecord *);
bool rec_has_added_size_4(struct KcfRecord *);

#endif
