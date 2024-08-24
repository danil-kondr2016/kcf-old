#pragma once
#ifndef _KCFERR_H_
#define _KCFERR_H_

#include <stdio.h>

enum KcfError {
	KCF_ERROR_OK,
	KCF_ERROR_UNKNOWN,
	KCF_ERROR_NOT_IMPLEMENTED,
	KCF_ERROR_INVALID_PARAMETER,
	KCF_ERROR_INVALID_FORMAT,
	KCF_ERROR_INVALID_DATA,
	KCF_ERROR_INVALID_STATE,
	KCF_ERROR_FILE_NOT_FOUND,
	KCF_ERROR_ACCESS_DENIED,
	KCF_ERROR_OUT_OF_MEMORY,
	KCF_ERROR_WRITE,
	KCF_ERROR_READ,
	KCF_ERROR_EOF,
	KCF_ERROR_PREMATURE_EOF,

	KCF_ERROR_MAX
};
typedef enum KcfError KCFERROR;

enum KcfFileSituation {
	KCF_SITUATION_READING_IN_BEGINNING,
	KCF_SITUATION_READING_IN_MIDDLE,
	KCF_SITUATION_WRITING,
};

KCFERROR ErrnoToKcf();
KCFERROR FileErrorToKcf(FILE *File, enum KcfFileSituation Situation);
const char *GetKcfErrorString(KCFERROR Error);

#endif
