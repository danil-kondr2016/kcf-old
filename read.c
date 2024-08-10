#include "record.h"
#include "utils.h"

#include "kcf_impl.h"
#include <stdlib.h>

static
KCFERROR read_record_header(
		HKCF hKCF, 
		struct KcfRecordHeader *RecordHdr,
		size_t *HeaderSize
)
{
	uint8_t buffer[14] = {0};
	size_t hdr_size = 0;
	size_t n_read;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!RecordHdr)
		return KCF_ERROR_INVALID_PARAMETER;

	n_read = fread(buffer, 1, 6, hKCF->File);
	if (n_read == 0)
		return FileErrorToKcf(hKCF->File, 
			KCF_SITUATION_READING_IN_BEGINNING);
	if (n_read < 6)
		return FileErrorToKcf(hKCF->File,
			KCF_SITUATION_READING_IN_MIDDLE);

	hdr_size = 6;
	RecordHdr->HeadCRC = read_u16le(buffer);
	RecordHdr->HeadType = buffer[2];
	RecordHdr->HeadFlags = buffer[3];
	RecordHdr->HeadSize = read_u16le(buffer+4);

	if ((RecordHdr->HeadFlags & KCF_HAS_ADDED_SIZE_4) != 0) {
		n_read = fread(buffer+6, 1, 4, hKCF->File);
		if (n_read < 4)
			return FileErrorToKcf(hKCF->File,
				KCF_SITUATION_READING_IN_MIDDLE);
		hdr_size += 4;
	}
	else {
		RecordHdr->AddedSize = 0;
	}

	if ((RecordHdr->HeadFlags & KCF_HAS_ADDED_SIZE_8)
			== KCF_HAS_ADDED_SIZE_8) {
		n_read = fread(buffer+10, 1, 4, hKCF->File);
		if (n_read < 4)
			return FileErrorToKcf(hKCF->File,
				KCF_SITUATION_READING_IN_MIDDLE);
		RecordHdr->AddedSize = read_u64le(buffer+6);
		hdr_size += 4;
	}
	else if ((RecordHdr->HeadFlags & KCF_HAS_ADDED_SIZE_8)
		       == KCF_HAS_ADDED_SIZE_4) {
		RecordHdr->AddedSize = read_u32le(buffer+6);
	}

	if (HeaderSize)
		*HeaderSize = hdr_size;
	return KCF_ERROR_OK;
}

KCFERROR ReadRecord(HKCF hKCF, struct KcfRecord *Record)
{
	KCFERROR Error;
	size_t HeaderSize;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!Record)
		return KCF_ERROR_INVALID_PARAMETER;

	Error = read_record_header(hKCF, &Record->Header, &HeaderSize);
	if (Error)
		return Error;
	Record->DataSize = Record->Header.HeadSize - HeaderSize;

	Record->Data = malloc(Record->DataSize);
	if (!Record->Data)
		return KCF_ERROR_OUT_OF_MEMORY;

	if (!fread(Record->Data, Record->DataSize, 1, hKCF->File))
		return KCF_ERROR_READ;

	hKCF->AvailableAddedData = Record->Header.AddedSize;

	return KCF_ERROR_OK;
}

KCFERROR SkipRecord(HKCF hKCF)
{
	struct KcfRecordHeader Header;
	size_t HeaderSize;
	uint64_t DataSize;
	KCFERROR Error;

	Error = read_record_header(hKCF, &Header, &HeaderSize);
	if (Error)
		return Error;

	DataSize = Header.HeadSize - HeaderSize + Header.AddedSize;
	if (kcf_fskip(hKCF->File, DataSize) == -1)
		return KCF_ERROR_READ;

	return KCF_ERROR_OK;
}

bool IsAddedDataAvailable(HKCF hKCF)
{
	if (!hKCF)
		return false;

	return hKCF->AvailableAddedData > 0;
}

KCFERROR ReadAddedData(
	HKCF hKCF, 
	void *Destination, 
	size_t BufferSize,
	size_t *BytesRead
)
{
	size_t n_read;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!Destination)
		return KCF_ERROR_INVALID_PARAMETER;

	if (BytesRead)
		*BytesRead = 0;

	if (hKCF->AvailableAddedData == 0) {
		return KCF_ERROR_OK;
	}

	if (hKCF->AvailableAddedData < BufferSize)
		BufferSize = hKCF->AvailableAddedData;

	n_read = fread(Destination, 1, BufferSize, hKCF->File);
	if (n_read < BufferSize)
		return KCF_ERROR_READ;

	hKCF->AvailableAddedData -= BufferSize;
	if (BytesRead)
		*BytesRead = n_read;

	return KCF_ERROR_OK;
}
