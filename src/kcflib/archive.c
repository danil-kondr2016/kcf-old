#include "stdio64.h"
#include "archive.h"

#include <stdlib.h>

#include "kcf_impl.h"

KCFERROR CreateArchive(char *Path, int Mode, PHKCF phKCF)
{
	HKCF Result = NULL;
	char *ModeString = NULL;
	int KcfModeValue = 0;

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

	Result->File = kcf_fopen(Path, ModeString);
	if (!Result->File) {
		return ErrnoToKcf();
	}

	if (Mode == KCF_MODE_CREATE) {
		Result->IsWriting = true;
		Result->WriterState = KCF_STATE_WRITING_MARKER;
	}
	else {
		Result->ReaderState = KCF_STATE_SEEKING_MARKER;
	}

	if (Mode == KCF_MODE_MODIFY || Mode == KCF_MODE_CREATE)
		Result->IsWritable = true;
	else
	 	Result->IsWritable = false;

	*phKCF = Result;
	return KCF_ERROR_OK;
}

void CloseArchive(HKCF hKCF)
{
	fclose(hKCF->File);
	free(hKCF);
}

bool StartReadingFromArchive(HKCF hKCF)
{
	if (hKCF->WriterState == KCF_STATE_WRITING_ADDED_DATA)
		FinishAddedData(hKCF);
	fflush(hKCF->File);

	hKCF->IsWriting = false;
	hKCF->ReaderState = KCF_STATE_READING_IDLE;
	return true;
}

bool StartWritingToArchive(HKCF hKCF)
{
	if (!hKCF->IsWritable)
		return false;

	hKCF->IsWriting = true;
	hKCF->WriterState = KCF_STATE_WRITING_IDLE;
	return true;
}
