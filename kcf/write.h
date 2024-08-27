#pragma once
#ifndef _WRITE_H_
#define _WRITE_H_

#include <stdbool.h>
#include <stdint.h>

#include <kcf/errors.h>

#include "record.h"

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
