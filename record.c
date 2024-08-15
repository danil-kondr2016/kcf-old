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

uint16_t CalculateRecordCRC(struct KcfRecord *Record)
{
	uint32_t CRC = 0;
	uint16_t CRC16 = 0;
	uint8_t Buffer[16] = {0};
	ptrdiff_t Offset = 0;

	WriteU8(Buffer, sizeof(Buffer), &Offset, Record->Header.HeadType);
	WriteU8(Buffer, sizeof(Buffer), &Offset, Record->Header.HeadFlags);
	WriteU16LE(Buffer, sizeof(Buffer), &Offset, Record->Header.HeadSize);

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
	return CRC16;
}

bool ValidateRecord(struct KcfRecord *Record)
{
	return CalculateRecordCRC(Record) == Record->Header.HeadCRC;
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

bool HasAddedSize8(struct KcfRecord *Record)
{
	return Record 
		&& ((Record->Header.HeadFlags & KCF_HAS_ADDED_SIZE_8) == KCF_HAS_ADDED_SIZE_8);
}

bool HasAddedSize4(struct KcfRecord *Record)
{
	return Record
		&& ((Record->Header.HeadFlags & KCF_HAS_ADDED_SIZE_8) == KCF_HAS_ADDED_SIZE_4);
}

bool HasAddedDataCRC32(struct KcfRecord *Record)
{
	return Record
		&& ((Record->Header.HeadFlags & KCF_HAS_ADDED_DATA_CRC32) != 0);
}

bool RecordToBuffer(
	struct KcfRecord *Record,
	uint8_t *Buffer,
	size_t Size
)
{
	ptrdiff_t Offset = 0;
	bool result = true;

	if (Size < Record->Header.HeadSize)
		return false;

	result = result && WriteU16LE(Buffer, Size, &Offset, Record->Header.HeadCRC);
	result = result && WriteU8(Buffer, Size, &Offset, Record->Header.HeadType);
	result = result && WriteU8(Buffer, Size, &Offset, Record->Header.HeadFlags);
	result = result && WriteU16LE(Buffer, Size, &Offset, Record->Header.HeadSize);

	if (HasAddedSize8(Record))
		result = result && WriteU64LE(Buffer, Size, &Offset, Record->Header.AddedSize);
	else if (HasAddedSize4(Record))
		result = result && WriteU32LE(Buffer, Size, &Offset, Record->Header.AddedSize);

	if (HasAddedDataCRC32(Record))	
		result = result && WriteU32LE(Buffer, Size, &Offset, Record->Header.AddedDataCRC32);

	memcpy(Buffer + Offset, Size - Offset, Record->DataSize);
	return result;	
}
