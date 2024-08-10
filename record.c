#include "record.h"
#include "utils.h"
#include "crc32c.h"

#include <stdlib.h>

void ClearRecord(struct KcfRecord *Record)
{
	if (Record->Data)
		free(Record->Data);
	Record->Data = NULL;
	Record->DataSize = 0;

	Record->Header.HeadCRC = 0;
	Record->Header.HeadType = 0;
	Record->Header.HeadFlags = 0;
	Record->Header.HeadSize = 0;
	Record->Header.AddedSize = 0;
}

bool ValidateRecord(struct KcfRecord *Record)
{
	uint32_t CRC = 0;
	uint16_t CRC16 = 0;
	uint8_t Buffer[12] = {0};
	size_t HeaderSize = 0;

	Buffer[0] = Record->Header.HeadType;
	Buffer[1] = Record->Header.HeadFlags;
	Buffer[2] = (Record->Header.HeadSize) & 0xFF;
	Buffer[3] = ((Record->Header.HeadSize) >> 8) & 0xFF;

	HeaderSize = 4;
	switch (Record->Header.HeadFlags & KCF_HAS_ADDED_SIZE_8) {
	case KCF_HAS_ADDED_SIZE_8:
		Buffer[11] = ((Record->Header.AddedSize) >> 56) & 0xFF;
		Buffer[10] = ((Record->Header.AddedSize) >> 48) & 0xFF;
		Buffer[9]  = ((Record->Header.AddedSize) >> 40) & 0xFF;
		Buffer[8]  = ((Record->Header.AddedSize) >> 32) & 0xFF;
		HeaderSize += 4;
	case KCF_HAS_ADDED_SIZE_4:
		Buffer[7] = ((Record->Header.AddedSize) >> 24) & 0xFF;
		Buffer[6] = ((Record->Header.AddedSize) >> 16) & 0xFF;
		Buffer[5] = ((Record->Header.AddedSize) >> 8)  & 0xFF;
		Buffer[4] = Record->Header.AddedSize & 0xFF;
		HeaderSize += 4;
		break;
	default:
		break;
	}

	CRC = crc32c(CRC, Buffer, HeaderSize);
	if (Record->Data && Record->DataSize > 0)
		CRC = crc32c(CRC, Record->Data, Record->DataSize);
	CRC16 = CRC & 0xFFFF;
	return (CRC16 == Record->Header.HeadCRC);
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
