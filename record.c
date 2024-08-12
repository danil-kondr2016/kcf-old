#include "record.h"
#include "utils.h"
#include "crc32c.h"

#include <stdlib.h>
#include <string.h>

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
	Record->Header.AddedDataCRC32 = 0;
}

/**
 * TODO create functions write_u16le, write_u32le, write_u64le as
 * read_u*le ones
 */
bool ValidateRecord(struct KcfRecord *Record)
{
	uint32_t CRC = 0;
	uint16_t CRC16 = 0;
	uint8_t Buffer[16] = {0};
	uint8_t *PBuf;
	size_t HeaderSize = 0;

	PBuf = Buffer;
	*PBuf++ = Record->Header.HeadType;
	*PBuf++ = Record->Header.HeadFlags;
	*PBuf++ = (Record->Header.HeadSize) & 0xFF;
	*PBuf++ = ((Record->Header.HeadSize) >> 8) & 0xFF;

	HeaderSize = 4;
	switch (Record->Header.HeadFlags & KCF_HAS_ADDED_SIZE_8) {
	case KCF_HAS_ADDED_SIZE_4:
		PBuf[0] = Record->Header.AddedSize & 0xFF;
		PBuf[1] = ((Record->Header.AddedSize) >> 8)  & 0xFF;
		PBuf[2] = ((Record->Header.AddedSize) >> 16) & 0xFF;
		PBuf[3] = ((Record->Header.AddedSize) >> 24) & 0xFF;
		HeaderSize += 4;
		PBuf += 4;
		break;
	case KCF_HAS_ADDED_SIZE_8:
		PBuf[0] = Record->Header.AddedSize & 0xFF;
		PBuf[1] = ((Record->Header.AddedSize) >> 8)  & 0xFF;
		PBuf[2] = ((Record->Header.AddedSize) >> 16) & 0xFF;
		PBuf[3] = ((Record->Header.AddedSize) >> 24) & 0xFF;
		PBuf[4] = ((Record->Header.AddedSize) >> 32) & 0xFF;
		PBuf[5] = ((Record->Header.AddedSize) >> 40) & 0xFF;
		PBuf[6] = ((Record->Header.AddedSize) >> 48) & 0xFF;
		PBuf[7] = ((Record->Header.AddedSize) >> 56) & 0xFF;
		HeaderSize += 8;
		PBuf += 8;
		break;
	default:
		break;
	}

	if (Record->Header.HeadFlags & KCF_HAS_ADDED_DATA_CRC32) {
		PBuf[0] = Record->Header.AddedDataCRC32 & 0xFF;
		PBuf[1] = ((Record->Header.AddedDataCRC32) >> 8) & 0xFF;
		PBuf[2] = ((Record->Header.AddedDataCRC32) >> 16) & 0xFF;
		PBuf[3] = ((Record->Header.AddedDataCRC32) >> 24) & 0xFF;
		HeaderSize += 4;
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

	if (!ValidateRecord(Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadType != KCF_ARCHIVE_HEADER)
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadSize < 8)
		return KCF_ERROR_INVALID_DATA;

	Header->ArchiveVersion = read_u16le(Record->Data);
	return KCF_ERROR_OK;
}
