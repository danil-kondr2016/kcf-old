#pragma once
#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <stdint.h>

enum KcfError
{
	KCF_ERROR_OK,
	KCF_ERROR_UNKNOWN,
	KCF_ERROR_NOT_IMPLEMENTED,
	KCF_ERROR_INVALID_PARAMETER,
	KCF_ERROR_INVALID_FORMAT,
	KCF_ERROR_FILE_NOT_FOUND,
	KCF_ERROR_ACCESS_DENIED,
	KCF_ERROR_OUT_OF_MEMORY,
	KCF_ERROR_WRITE,
	KCF_ERROR_READ,
	KCF_ERROR_EOF,
};
typedef enum KcfError KCFERROR;

KCFERROR ErrnoToKcf();

#endif
