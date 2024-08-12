#include "record.h"
#include "utils.h"
#include "bytepack.h"
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

bool ValidateRecord(struct KcfRecord *Record)
{
	uint32_t CRC = 0;
	uint16_t CRC16 = 0;
	uint8_t Buffer[16] = {0};
	ptrdiff_t Offset = 0;

	WriteU8(Buffer, sizeof(Buffer), &Offset, Record->Header.HeadType);
	WriteU8(Buffer, sizeof(Buffer), &Offset, Record->Header.HeadFlags);
	WriteU16LE(Buffer, sizeof(Buffer), &Offset, Record->Header.HeadSize);

	HeaderSize = 4;
	switch (Record->Header.HeadFlags & KCF_HAS_ADDED_SIZE_8) {
	case KCF_HAS_ADDED_SIZE_4:
		WriteU32LE(Buffer, sizeof(Buffer), &Offset, 
				Record->Header.AddedSize);
		break;
	case KCF_HAS_ADDED_SIZE_8:
		WriteU64LE(Buffer, sizeof(Buffer), &Offset,
				Record->Header.AddedSize);
		break;
	}

	if (Record->Header.HeadFlags & KCF_HAS_ADDED_DATA_CRC32) {
		WriteU32LE(Buffer, sizeof(Buffer), &Offset,
				Record->Header.AddedDataCRC32);
	}

	CRC = crc32c(CRC, Buffer, Offset);
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

	ReadU16LE(Record->Data, Record->DataSize, NULL, 
			&Header->ArchiveVersion);
	return KCF_ERROR_OK;
}
