#pragma once
#ifndef _READ_H_
#define _READ_H_

#include <stdbool.h>
#include <stdint.h>

#include <kcf/errors.h>
#include "record.h"

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

#endif
