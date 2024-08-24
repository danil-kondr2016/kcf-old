#include <kcf/archive.h>
#include <kcf/errors.h>

#include <stdlib.h>

#include "kcf_impl.h"

KCFERROR KCF_create(BIO *stream, KCF **pkcf)
{
	KCF *result;
	if (!pkcf || !stream)
		return KCF_ERROR_INVALID_PARAMETER;

	result = calloc(1, sizeof(struct kcf_st));
	if (!result) {
		return KCF_ERROR_OUT_OF_MEMORY;
	}

	result->Stream = stream;
	if (!BIO_should_write(result->Stream)) {
		result->IsWritable = true;
	}

	*pkcf = result;
	return KCF_ERROR_OK;
}

void KCF_close(KCF *kcf)
{
	free(kcf);
}

bool KCF_start_reading(KCF *kcf)
{
	if (kcf->WriterState == KCF_WRSTATE_ADDED_DATA)
		KCF_finish_added_data(kcf);
	BIO_flush(kcf->Stream);

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
