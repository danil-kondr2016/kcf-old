#include "record.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

KCFERROR RecordToFileHeader(
	struct KcfRecord *Record,
	struct KcfFileHeader *Header
)
{
	uint8_t *pbuf;
	if (!Record)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!Header)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!ValidateRecord(Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadType != KCF_FILE_HEADER)
		return KCF_ERROR_INVALID_DATA;

	pbuf = Record->Data;
	Header->FileFlags = *pbuf++;
	Header->FileType = *pbuf++;

	switch (Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) {
	case KCF_FILE_HAS_UNPACKED_8:
		Header->UnpackedSize = read_u64le(pbuf);
		pbuf += 8;
		break;
	case KCF_FILE_HAS_UNPACKED_4:
		Header->UnpackedSize = read_u32le(pbuf);
		pbuf += 4;
		break;
	}

	if (Header->FileFlags & KCF_FILE_HAS_FILE_CRC32) {
		Header->FileCRC32 = read_u32le(pbuf);
		pbuf += 4;
	}

	Header->CompressionInfo = read_u32le(pbuf); pbuf += 4;

	if (Header->FileFlags & KCF_FILE_HAS_TIMESTAMP) {
		Header->TimeStamp = read_u64le(pbuf);
		pbuf += 8;
	}

	Header->FileNameSize = read_u16le(pbuf); pbuf += 2;
	Header->FileName = malloc(Header->FileNameSize + 1);
	if (!Header->FileName) {
		return KCF_ERROR_OUT_OF_MEMORY;
	}
	Header->FileName[Header->FileNameSize] = 0;
	memcpy(Header->FileName, pbuf, Header->FileNameSize);

	return KCF_ERROR_OK;
}

void ClearFileHeader(struct KcfFileHeader *Header)
{
	if (Header->FileName)
		free(Header->FileName);

	memset(Header, 0, sizeof(*Header));
}
