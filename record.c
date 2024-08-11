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

	if (!ValidateRecord(&Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadType != KCF_ARCHIVE_HEADER)
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadSize < 8)
		return KCF_ERROR_INVALID_DATA;

	Header->ArchiveVersion = read_u16le(Record->Data);
	return KCF_ERROR_OK;
}

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

	if (!ValidateRecord(&Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadType != KCF_FILE_HEADER)
		return KCF_ERROR_INVALID_DATA;

	pbuf = Record->Data;
	Header->FileFlags = *pbuf++;
	switch (Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) {
	case KCF_FILE_HAS_UNPACKED_8:
		Header->CompressedSize = read_u64le(pbuf);
		pbuf += 8;
		break;
	case KCF_FILE_HAS_UNPACKED_4:
		Header->CompressedSize = read_u32le(pbuf);
		pbuf += 4;
		break;
	}

	if (Header->FileFlags & KCF_FILE_HAS_FILE_CRC32) {
		Header->FileCRC32 = read_u32le(pbuf);
		pbuf += 4;
	}

	Header->FileType = read_u32le(pbuf); pbuf += 4;
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
