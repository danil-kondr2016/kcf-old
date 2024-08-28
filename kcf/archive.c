#include <kcf/archive.h>
#include <kcf/errors.h>

#include <stdlib.h>

#include "kcf_impl.h"

KCFERROR KCF_create(IO *stream, KCF **pkcf)
{
	KCF *result;
	if (!pkcf || !stream)
		return KCF_ERROR_INVALID_PARAMETER;

	result = calloc(1, sizeof(struct kcf_st));
	if (!result) {
		return KCF_ERROR_OUT_OF_MEMORY;
	}

	result->Stream = stream;

	*pkcf = result;
	return KCF_ERROR_OK;
}

void KCF_close(KCF *kcf)
{
	free(kcf);
}

KCFERROR KCF_start_reading(KCF *kcf)
{
	KCFERROR Error;
	
	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;

	switch (kcf->ParserState) {
	case KCF_PSTATE_NEUTRAL:
	case KCF_PSTATE_READING:
		kcf->ParserState = KCF_PSTATE_READ_MARKER;
		break;
	case KCF_PSTATE_WRITE_MARKER:
		kcf->ParserState = KCF_PSTATE_READING;
		break;
	case KCF_PSTATE_WRITE_ADDED_DATA:
		Error = KCF_finish_added_data(kcf);
		if (Error)
			return Error;
	case KCF_PSTATE_WRITE_RECORD:
		if (IO_flush(kcf->Stream) < 0)
			return KCF_ERROR_UNKNOWN;
		kcf->ParserState = KCF_PSTATE_READ_RECORD_HEADER;
		break;
	}

	return KCF_ERROR_OK;
}

KCFERROR KCF_start_writing(KCF *kcf)
{
	KCFERROR Error;

	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;

	switch (kcf->ParserState) {
	case KCF_PSTATE_NEUTRAL:
	case KCF_PSTATE_READING:
	case KCF_PSTATE_WRITING:
	case KCF_PSTATE_READ_MARKER:
		kcf->ParserState = KCF_PSTATE_WRITE_MARKER;
		Error = KCF_write_marker(kcf);
		if (Error)
			return Error;
		break;
	case KCF_PSTATE_READ_RECORD_HEADER:
		kcf->ParserState = KCF_PSTATE_WRITE_RECORD;
		break;
	case KCF_PSTATE_READ_ADDED_DATA:
	case KCF_PSTATE_READ_RECORD_DATA:
		return KCF_ERROR_INVALID_STATE;
	}

	return KCF_ERROR_OK;
}

KCFERROR KCF_init_archive(KCF *kcf)
{
	KCFERROR Error;
	
	if ((Error = KCF_start_writing(kcf)))
		return Error;

	if ((Error = KCF_write_archive_header(kcf, 0)))
		return Error;

	return KCF_ERROR_OK;
}
