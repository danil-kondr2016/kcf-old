#include "record.h"
#include "utils.h"

#include <stdlib.h>

void ClearRecord(struct KcfRecord *Record)
{
	free(Record->Data);
	Record->Data = NULL;
	Record->DataSize = 0;

	Record->Header.HeadCRC = 0;
	Record->Header.HeadType = 0;
	Record->Header.HeadFlags = 0;
	Record->Header.HeadSize = 0;
	Record->Header.AddedSize = 0;
}

KCFERROR RecordToArchiveHeader(
	struct KcfRecord *Record,
	struct KcfArchiveHeader *Header
)
{
	if (!Record)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!Header)
		return KCF_ERROR_INVALID_PARAMETER;

	if (Record->Header.HeadType != KCF_ARCHIVE_HEADER)
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadSize < 8)
		return KCF_ERROR_INVALID_DATA;

	Header->ArchiveVersion = read_u16le(Record->Data);
	return KCF_ERROR_OK;
}
