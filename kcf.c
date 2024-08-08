#include "kcf.h"

#include <errno.h>
#include <stdlib.h>

struct Kcf
{
	FILE *File;
};

HRESULT CreateArchive(char *Path, int Mode, PHKCF phKCF)
{
	HKCF Result = NULL;
	char *ModeString = NULL;

	if (!phKCF)
		return KCF_RESULT_INVALID_PARAMETER;

	if (!Path)
		return KCF_RESULT_INVALID_PARAMETER;

	switch (Mode) {
	case KCF_MODE_READ:
		ModeString = "rb";
		break;
	case KCF_MODE_CREATE:
		ModeString = "w+b";
		break;
	case KCF_MODE_MODIFY:
		ModeString = "r+b";
		break;
	default:
		return KCF_RESULT_INVALID_PARAMETER;
	}

	Result = calloc(sizeof(struct Kcf), 1);
	if (!Result)
		return KCF_RESULT_OUTOFMEMORY;

	Result->File = fopen(Path, ModeString);
	if (!Result->File) {
		int Error = errno;
		switch (Error) {
		case EINVAL:
			return KCF_RESULT_INVALID_PARAMETER;
		case ENOMEM:
			return KCF_RESULT_OUTOFMEMORY;
		case EACCES:
		case EPERM:
			return KCF_RESULT_ACCESS_DENIED;
		case ENOENT:
			return KCF_RESULT_FILE_NOT_FOUND;
		case EROFS:
			return KCF_RESULT_WRITE_PROTECT;
		default:
			return UINT32_C(0x80000000);
		}
	}

	*phKCF = Result;
	return KCF_RESULT_OK;
}

void CloseArchive(HKCF hKCF)
{
	fclose(hKCF->File);
	free(hKCF);
}
