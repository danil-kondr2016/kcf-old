#pragma once
#ifndef _PACK_H_
#define _PACK_H_

#include "archive.h"
#include "errors.h"

#define KCF_PACKING_JUST_STORE 0
KCFERROR PackFile(HKCF hKCF, char *FileName, int PackingMode);

#endif