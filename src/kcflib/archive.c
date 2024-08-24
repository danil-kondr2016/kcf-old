#include <kcf/archive.h>
#include <kcf/errors.h>

#include <stdlib.h>

#include "kcf_impl.h"
#include "stdio64.h"

KCFERROR KCF_create(char *Path, int Mode, KCF **pkcf)
{
	KCF *result    = NULL;
	char *mode_str = NULL;

	if (!pkcf)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!Path)
		return KCF_ERROR_INVALID_PARAMETER;

	switch (Mode) {
	case KCF_MODE_READ:
		mode_str = "rb";
		break;
	case KCF_MODE_CREATE:
		mode_str = "w+b";
		break;
	case KCF_MODE_MODIFY:
		mode_str = "r+b";
		break;
	default:
		return KCF_ERROR_INVALID_PARAMETER;
	}

	result = calloc(sizeof(struct kcf_st), 1);
	if (!result)
		return KCF_ERROR_OUT_OF_MEMORY;

	result->File = kcf_fopen(Path, mode_str);
	if (!result->File) {
		return kcf_errno();
	}

	if (Mode == KCF_MODE_CREATE) {
		result->IsWriting   = true;
		result->WriterState = KCF_WRSTATE_MARKER;
	} else {
		result->ReaderState = KCF_RDSTATE_MARKER;
	}

	if (Mode == KCF_MODE_MODIFY || Mode == KCF_MODE_CREATE)
		result->IsWritable = true;
	else
		result->IsWritable = false;

	*pkcf = result;
	return KCF_ERROR_OK;
}

void KCF_close(KCF *kcf)
{
	fclose(kcf->File);
	free(kcf);
}

bool KCF_start_reading(KCF *kcf)
{
	if (kcf->WriterState == KCF_WRSTATE_ADDED_DATA)
		KCF_finish_added_data(kcf);
	fflush(kcf->File);

	kcf->IsWriting   = false;
	kcf->ReaderState = KCF_RDSTATE_IDLE;
	return true;
}

bool KCF_start_writing(KCF *kcf)
{
	if (!kcf->IsWritable)
		return false;

	kcf->IsWriting   = true;
	kcf->WriterState = KCF_WRSTATE_IDLE;
	return true;
}
