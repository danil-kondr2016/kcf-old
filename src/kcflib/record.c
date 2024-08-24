#include <kcf/record.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bytepack.h"
#include "crc32c.h"

void rec_clear(struct KcfRecord *Record)
{
	assert(Record);

	if (Record->Data)
		free(Record->Data);
	Record->Data     = NULL;
	Record->DataSize = 0;

	Record->HeadCRC        = 0;
	Record->HeadType       = 0;
	Record->HeadFlags      = 0;
	Record->HeadSize       = 0;
	Record->AddedSize      = 0;
	Record->AddedDataCRC32 = 0;
}

uint16_t rec_calculate_CRC(struct KcfRecord *Record)
{
	uint32_t CRC       = 0;
	uint16_t CRC16     = 0;
	uint8_t Buffer[16] = {0};
	ptrdiff_t Offset   = 0;

	assert(Record);

	WriteU8(Buffer, sizeof(Buffer), &Offset, Record->HeadType);
	WriteU8(Buffer, sizeof(Buffer), &Offset, Record->HeadFlags);
	WriteU16LE(Buffer, sizeof(Buffer), &Offset, Record->HeadSize);

	switch (Record->HeadFlags & KCF_HAS_ADDED_SIZE_8) {
	case KCF_HAS_ADDED_SIZE_4:
		WriteU32LE(Buffer, sizeof(Buffer), &Offset,
		           Record->AddedSize);
		break;
	case KCF_HAS_ADDED_SIZE_8:
		WriteU64LE(Buffer, sizeof(Buffer), &Offset,
		           Record->AddedSize);
		break;
	}

	if (Record->HeadFlags & KCF_HAS_ADDED_DATA_CRC32) {
		WriteU32LE(Buffer, sizeof(Buffer), &Offset,
		           Record->AddedDataCRC32);
	}

	CRC = crc32c(CRC, Buffer, Offset);
	if (Record->Data && Record->DataSize > 0)
		CRC = crc32c(CRC, Record->Data, Record->DataSize);
	CRC16 = CRC & 0xFFFF;
	return CRC16;
}

bool rec_validate(struct KcfRecord *Record)
{
	return Record && (rec_calculate_CRC(Record) == Record->HeadCRC);
}

bool rec_has_added_size_8(struct KcfRecord *Record)
{
	return Record && ((Record->HeadFlags & KCF_HAS_ADDED_SIZE_8) ==
	                  KCF_HAS_ADDED_SIZE_8);
}

bool rec_has_added_size_4(struct KcfRecord *Record)
{
	return Record && ((Record->HeadFlags & KCF_HAS_ADDED_SIZE_8) ==
	                  KCF_HAS_ADDED_SIZE_4);
}

bool rec_to_buffer(struct KcfRecord *Record, uint8_t *Buffer, size_t Size)
{
	ptrdiff_t Offset = 0;
	bool result      = true;

	if (Size < Record->HeadSize)
		return false;

	result =
	    result && WriteU16LE(Buffer, Size, &Offset, Record->HeadCRC);
	result =
	    result && WriteU8(Buffer, Size, &Offset, Record->HeadType);
	result =
	    result && WriteU8(Buffer, Size, &Offset, Record->HeadFlags);
	result = result &&
	         WriteU16LE(Buffer, Size, &Offset, Record->HeadSize);

	if (rec_has_added_size_8(Record))
		result = result && WriteU64LE(Buffer, Size, &Offset,
		                              Record->AddedSize);
	else if (rec_has_added_size_4(Record))
		result = result && WriteU32LE(Buffer, Size, &Offset,
		                              Record->AddedSize);

	if (rec_has_added_data_CRC(Record))
		result = result && WriteU32LE(Buffer, Size, &Offset,
		                              Record->AddedDataCRC32);

	memcpy(Buffer + Offset, Record->Data, Record->DataSize);
	return result;
}

void rec_fix(struct KcfRecord *Record)
{
	assert(Record);

	Record->HeadSize = 6 + Record->DataSize;
	switch (Record->HeadFlags & KCF_HAS_ADDED_SIZE_8) {
	case KCF_HAS_ADDED_SIZE_4:
		Record->HeadSize += 4;
		break;
	case KCF_HAS_ADDED_SIZE_8:
		Record->HeadSize += 8;
		break;
	}

	if ((Record->HeadFlags & KCF_HAS_ADDED_DATA_CRC32)) {
		Record->HeadSize += 4;
	}

	Record->HeadCRC = rec_calculate_CRC(Record);
}

KCFERROR rec_copy(struct KcfRecord *Destination, struct KcfRecord *Source)
{
	assert(Destination);
	assert(Source);

	Destination->HeadCRC = Source->HeadCRC;
	Destination->HeadFlags = Source->HeadFlags;
	Destination->HeadType = Source->HeadType;
	Destination->HeadSize = Source->HeadSize;

	Destination->AddedSize = Source->AddedSize;
	Destination->AddedDataCRC32 = Source->AddedDataCRC32;
	
	Destination->DataSize = Source->DataSize;
	if (Source->Data) {
		Destination->Data = malloc(Source->DataSize);
		if (!Destination->Data)
			return KCF_ERROR_OUT_OF_MEMORY;

		memcpy(Destination->Data, Source->Data, Source->DataSize);
	} else {
		Destination->Data = NULL;
	}

	return KCF_ERROR_OK;
}
