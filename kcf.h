#pragma once
#ifndef _KCF_H_
#define _KCF_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "record.h"
#include "errors.h"

typedef struct Kcf *HKCF, **PHKCF;

#define KCF_MODE_READ   0x01
#define KCF_MODE_CREATE 0x02
#define KCF_MODE_MODIFY 0x03

KCFERROR CreateArchive(char *Path, int Mode, PHKCF phKCF);
void CloseArchive(HKCF hKCF);

/* Write/read mode functions */

/**
 * Switches to write mode. Won't work if file is not open for writing 
 * or modifying.
 */
bool StartWritingToArchive(HKCF hKCF);

/**
 * Switches to read mode. If any added data have been written, it will 
 * be finished before switching to read mode.
 */
bool StartReadingFromArchive(HKCF hKCF);

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

KCFERROR WriteArchiveHeader(HKCF hKCF, int Reserved);


#endif
