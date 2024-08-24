#include <kcf/errors.h>
#include <kcf/record.h>

#include <stdlib.h>
#include <string.h>

#include "bytepack.h"

KCFERROR rec_to_archive_header(struct KcfRecord *Record,
                               struct KcfArchiveHeader *Header)
{
	if (!Record)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!Header)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!rec_validate(Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->HeadType != KCF_ARCHIVE_HEADER)
		return KCF_ERROR_INVALID_DATA;
	if (Record->HeadSize < 8)
		return KCF_ERROR_INVALID_DATA;

	ReadU16LE(Record->Data, Record->DataSize, NULL,
	          &Header->ArchiveVersion);
	return KCF_ERROR_OK;
}

KCFERROR rec_from_archive_header(struct KcfArchiveHeader *Header,
                                 struct KcfRecord *Record)
{
	if (!Record || !Header)
		return KCF_ERROR_INVALID_PARAMETER;

	Record->Data = malloc(2);
	WriteU16LE(Record->Data, 2, NULL, Header->ArchiveVersion);
	Record->DataSize         = 2;
	Record->HeadFlags = 0;
	Record->HeadType  = KCF_ARCHIVE_HEADER;
	rec_fix(Record);

	return KCF_ERROR_OK;
}

void ahdr_clear(struct KcfArchiveHeader *Header)
{
	memset(Header, 0, sizeof(*Header));
}
