#include "record.h"
#include "bytepack.h"

#include <string.h>

KCFERROR RecordToArchiveHeader(
	struct KcfRecord *Record,
	struct KcfArchiveHeader *Header
)
{
	if (!Record)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!Header)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!ValidateRecord(Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadType != KCF_ARCHIVE_HEADER)
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadSize < 8)
		return KCF_ERROR_INVALID_DATA;

	ReadU16LE(Record->Data, Record->DataSize, NULL, 
			&Header->ArchiveVersion);
	return KCF_ERROR_OK;
}

void ClearArchiveHeader(struct KcfArchiveHeader *Header)
{
	memset(Header, 0, sizeof(*Header));
}
