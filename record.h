/**
 * \file record.h
 * 
 * Functions for working with records inisde KCF archive.
 */

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

struct KcfArchiveHeader
{
	uint16_t ArchiveVersion;
};

/* Read functions */

/**
 * \brief Scans archive for the KCF archive marker "KC!\x1A\6\0".
 */
KCFERROR ScanArchiveForMarker(HKCF hKCF);

/**
 * \brief Reads current record and saves its data to the structure.
 */
KCFERROR ReadRecord(HKCF hKCF, struct KcfRecord *Record);

/**
 * \brief Skips current record and goes into the next one.
 *
 * If main field of record has been read and the pointer of the file
 * stands at the added data field, added data field will be fully
 * discarded.
 */
KCFERROR SkipRecord(HKCF hKCF);

bool IsAddedDataAvailable(HKCF hKCF);
KCFERROR ReadAddedData(HKCF hKCF, void *Destination, size_t BufferSize,
		size_t *BytesRead);

/* Record manipulation functions */

uint16_t CalculateRecordCRC(struct KcfRecord *Record);
bool ValidateRecord(struct KcfRecord *Record);
void FixRecord(struct KcfRecord *Record);
void ClearRecord(struct KcfRecord *Record);
KCFERROR CopyRecord(struct KcfRecord *Destination, struct KcfRecord *Source);

bool RecordToBuffer(struct KcfRecord *Record, uint8_t *Buffer, size_t Size);

KCFERROR RecordToArchiveHeader(struct KcfRecord *, struct KcfArchiveHeader *);
KCFERROR ArchiveHeaderToRecord(struct KcfArchiveHeader *, struct KcfRecord *);
void ClearArchiveHeader(struct KcfArchiveHeader *);

KCFERROR RecordToFileHeader(struct KcfRecord *, struct KcfFileHeader *);
KCFERROR FileHeaderToRecord(struct KcfFileHeader *, struct KcfRecord *);
void ClearFileHeader(struct KcfFileHeader *);

/* Flag probing functions */

bool HasAddedSize8(struct KcfRecord *);
bool HasAddedSize4(struct KcfRecord *);
bool HasAddedDataCRC32(struct KcfRecord *);

/* Write functions */

/**
 * \brief Writes the KCF archive marker "KC!\x1A\6\0".
 * 
 * \param[in] hKCF handle to KCF archive
 * \return `KCF_ERROR_OK` if success, `KCF_ERROR_WRITE` if failed to write
 */
KCFERROR WriteArchiveMarker(HKCF hKCF);

/**
 * \brief Writes the record into KCF archive.
 *
 * If flag `KCF_HAS_ADDED_SIZE_8` or `KCF_HAS_ADDED_SIZE_4` has been set,
 * you can write added data into the record with function `WriteAddedData`.
 * You can also set flag `KCF_HAS_ADDED_SIZE_CRC32` to calculate CRC32
 * of record's added data during writing.
 *
 * If added data has not been finished, it will be finished when this 
 * function is called.
 */
KCFERROR WriteRecord(HKCF hKCF, struct KcfRecord *Record);

/**
 * \brief Writes the record into KCF archive with \p AddedData already
 * specified.
 *
 * If \p Size >= 2^31 - 1, flag `KCF_HAS_ADDED_SIZE_8` is set, otherwise
 * flag `KCF_HAS_ADDED_SIZE_4` is set. If flag `KCF_HAS_ADDED_SIZE_CRC32`
 * is set, CRC32 of added data will be written into the record.
 */
KCFERROR WriteRecordWithAddedData(HKCF hKCF, struct KcfRecord *Record, 
	uint8_t *AddedData, size_t Size);

/**
 * \brief Writes added data into the archive. Should be called after
 * `WriteRecord` call.
 */
KCFERROR WriteAddedData(HKCF hKCF, uint8_t *AddedData, size_t Size);

/**
 * \brief Finishes writing of added data into the archive. Patches 
 * forward pointers inside the header so this function will return
 * an error on non-seekable data streams. 
 */
KCFERROR FinishAddedData(HKCF hKCF);

#endif
