#pragma once
#ifndef _RECORD_H_
#define _RECORD_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "errors.h"

struct Kcf;
typedef struct Kcf *HKCF, **PHKCF;

#define KCF_MODE_READ   0x01
#define KCF_MODE_CREATE 0x02
#define KCF_MODE_MODIFY 0x03

KCFERROR CreateArchive(char *Path, int Mode, PHKCF phKCF);
void CloseArchive(HKCF hKCF);

#endif
