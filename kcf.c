#include "kcf.h"

#include <errno.h>
#include <stdlib.h>

#include "kcf_impl.h"

KCFERROR CreateArchive(char *Path, int Mode, PHKCF phKCF)
{
	HKCF Result = NULL;
	char *ModeString = NULL;

	if (!phKCF)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!Path)
		return KCF_ERROR_INVALID_PARAMETER;

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
		return KCF_ERROR_INVALID_PARAMETER;
	}

	Result = calloc(sizeof(struct Kcf), 1);
	if (!Result)
		return KCF_ERROR_OUT_OF_MEMORY;

	Result->File = fopen(Path, ModeString);
	if (!Result->File) {
		return ErrnoToKcf();
	}

	*phKCF = Result;
	return KCF_ERROR_OK;
}

void CloseArchive(HKCF hKCF)
{
	fclose(hKCF->File);
	free(hKCF);
}
