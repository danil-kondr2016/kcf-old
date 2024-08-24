#pragma once
#ifndef _ARCHIVE_H_
#define _ARCHIVE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "errors.h"
#include "record.h"

typedef struct kcf_st KCF;

#define KCF_MODE_READ   0x01
#define KCF_MODE_CREATE 0x02
#define KCF_MODE_MODIFY 0x03

KCFERROR KCF_create(char *Path, int Mode, KCF **pkcf);
void KCF_close(KCF *kcf);

/* Write/read mode functions */

/**
 * Switches to write mode. Won't work if file is not open for writing
 * or modifying.
 */
bool KCF_start_writing(KCF *kcf);

/**
 * Switches to read mode. If any added data have been written, it will
 * be finished before switching to read mode.
 */
bool KCF_start_reading(KCF *kcf);

/* Read functions */

/**
 * \brief Scans archive for the KCF archive marker "KC!\x1A\6\0".
 */
KCFERROR KCF_find_marker(KCF *kcf);

/**
 * \brief Reads current record and saves its data to the structure.
 */
KCFERROR KCF_read_record(KCF *kcf, struct KcfRecord *record);

/**
 * \brief Skips current record and goes into the next one.
 *
 * If main field of record has been read and the pointer of the file
 * stands at the added data field, added data field will be fully
 * discarded.
 */
KCFERROR KCF_skip_record(KCF *kcf);

bool KCF_is_added_data_available(KCF *kcf);
KCFERROR KCF_read_added_data(KCF *kcf, void *Destination, size_t BufferSize,
                             size_t *BytesRead);

/* Write functions */

/**
 * \brief Writes the KCF archive marker "KC!\x1A\6\0".
 *
 * \param[in] kcf handle to KCF archive
 * \return `KCF_ERROR_OK` if success, `KCF_ERROR_WRITE` if failed to write
 */
KCFERROR KCF_write_marker(KCF *kcf);

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
KCFERROR KCF_write_record(KCF *kcf, struct KcfRecord *Record);

/**
 * \brief Writes the record into KCF archive with \p AddedData already
 * specified.
 *
 * If \p Size >= 2^31 - 1, flag `KCF_HAS_ADDED_SIZE_8` is set, otherwise
 * flag `KCF_HAS_ADDED_SIZE_4` is set. If flag `KCF_HAS_ADDED_SIZE_CRC32`
 * is set, CRC32 of added data will be written into the record.
 */
KCFERROR KCF_write_record_with_added_data(KCF *kcf, struct KcfRecord *Record,
                                          uint8_t *AddedData, size_t Size);

/**
 * \brief Writes added data into the archive. Should be called after
 * `WriteRecord` call.
 */
KCFERROR KCF_write_added_data(KCF *kcf, uint8_t *AddedData, size_t Size);

/**
 * \brief Finishes writing of added data into the archive. Patches
 * forward pointers inside the header so this function will return
 * an error on non-seekable data streams.
 */
KCFERROR KCF_finish_added_data(KCF *kcf);

KCFERROR KCF_write_archive_header(KCF *kcf, int Reserved);

#endif
