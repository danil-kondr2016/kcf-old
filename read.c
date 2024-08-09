#include "record.h"
#include "utils.h"

#include "kcf_impl.h"
#include <stdlib.h>

static inline uint16_t read_u16le(uint8_t *buf)
{
	return buf[0] | (buf[1]<<8);
}

static inline uint32_t read_u32le(uint8_t *buf)
{
	return (uint32_t)read_u16le(buf) | ((uint32_t)read_u16le(buf+2) << 16);
}

static inline uint64_t read_u64le(uint8_t *buf) {
	return (uint64_t)read_u32le(buf)
		| ((uint64_t)read_u32le(buf+4) << 32);
}

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

#ifdef TEST_READ_RECORD
#include <string.h>
int main(void)
{
	HKCF hKCF;
	KCFERROR Error;
	struct KcfRecord Record;

	Error = CreateArchive("tests/mk1g.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		fputs("Failed to conduct test #3\n", stderr);
		return 1;
	}

	ScanArchiveForMarker(hKCF);

	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		fputs("ReadHeader,1,good,fail,",stdout);
		printf("%d\n", Error);
		return 0;
	}

	if (Record.Header.HeadCRC == 0xFFFF
			&& Record.Header.HeadType == 0x41
			&& Record.Header.HeadFlags == 0x00
			&& Record.Header.HeadSize == 0x0008
			&& Record.DataSize == 2
			&& Record.Data[0] == 0x01
			&& Record.Data[1] == 0x00)
	{
		puts("ReadHeader,1,good,pass");
	}
	else {
		fputs("ReadHeader,1,good,fail,",stdout);
		printf("CRC=%04X,Type=%02X,Flags=%02X,Size=%04X\n",
				Record.Header.HeadCRC,
				Record.Header.HeadType,
				Record.Header.HeadFlags,
				Record.Header.HeadSize);
	}
	CloseArchive(hKCF);
	free(Record.Data);

	Error = CreateArchive("tests/rh2g.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		fputs("Failed to conduct test #4\n", stderr);
		return 1;
	}

	ScanArchiveForMarker(hKCF);

	SkipRecord(hKCF);

	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		fputs("ReadHeader,2,good,fail,",stdout);
		printf("%d\n", Error);
		return 0;
	}

	if (Record.Header.HeadCRC == 0x0000
			&& Record.Header.HeadType == 0x30
			&& Record.Header.HeadFlags == 0x00
			&& Record.Header.HeadSize == 0x0010
			&& Record.DataSize == 10)
	{
		puts("ReadHeader,2,good,pass");
	}
	else {
		fputs("ReadHeader,2,good,fail,",stdout);
	}

	SkipRecord(hKCF);
	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		fputs("ReadHeader,3,good,fail,",stdout);
		printf("%d\n", Error);
		return 0;
	}

	if (Record.Header.HeadType == 0x32) {
		puts("ReadHeader,3,good,pass");
	}
	else {
		fputs("ReadHeader,3,good,fail\n",stdout);
	}

	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		fputs("ReadHeader,4,good,fail,",stdout);
		printf("%d\n", Error);
		return 0;
	}

	if (Record.Header.HeadType == 0x33 && Record.Header.AddedSize == 10) {
		puts("ReadHeader,4,good,pass");
	}
	else {
		fputs("ReadHeader,4,good,fail\n",stdout);
	}

	{
		uint8_t buf1[12];
		size_t n_read;

		Error = ReadAddedData(hKCF, buf1, 12, &n_read);
		if (Error) {
			puts("ReadAddedData,5,good,fail");
		}
		else if (n_read == 10 &&
				memcmp(buf1, "0123456789", 10) == 0)
		{
			puts("ReadAddedData,5,good,pass");
		}
	}

	CloseArchive(hKCF);


	return 0;
}
#endif
