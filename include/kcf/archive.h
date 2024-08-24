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

#endif
