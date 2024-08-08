#include "record.h"

#include "kcf_impl.h"

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

KCFERROR ReadRecordHeader(HKCF hKCF, struct KcfRecordHeader *RecordHdr)
{
	uint8_t buffer[14] = {0};
	size_t n_read = 0;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!RecordHdr)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!fread(buffer, 6, 1, hKCF->File))
		return KCF_ERROR_READ;
	RecordHdr->HeadCRC = read_u16le(buffer);
	RecordHdr->HeadType = buffer[2];
	RecordHdr->HeadFlags = buffer[3];
	RecordHdr->HeadSize = read_u16le(buffer+4);

	if (RecordHdr->HeadFlags & KCF_HAS_ADDED_SIZE_8 != 0) {
		if (!fread(buffer+6, 4, 1, hKCF->File))
			return KCF_ERROR_READ;
	}
	else {
		RecordHdr->AddedSize = 0;
	}

	if (RecordHdr->HeadFlags & KCF_HAS_ADDED_SIZE_8
			== KCF_HAS_ADDED_SIZE_8) {
		if (!fread(buffer+10, 4, 1, hKCF->File))
			return KCF_ERROR_READ;
		RecordHdr->AddedSize = read_u64le(buffer+6);
	}
	else if (RecordHdr->HeadFlags & KCF_HAS_ADDED_SIZE_8
		       == KCF_HAS_ADDED_SIZE_4) {
		RecordHdr->AddedSize = read_u32le(buffer+6);
	}

	return KCF_ERROR_OK;
}

#ifdef TEST_READ_RECORD_HEADER
int main(void)
{
	HKCF hKCF;
	KCFERROR Error;
	struct KcfRecordHeader Header;

	Error = CreateArchive("tests/mk1g.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		fputs("Failed to conduct test #3\n", stderr);
		return 1;
	}

	ScanArchiveForMarker(hKCF);

	Error = ReadRecordHeader(hKCF, &Header);
	if (Error) {
		puts("ReadHeader,1,good,fail");
		return 0;
	}

	if (Header.HeadCRC == 0xFFFF
			&& Header.HeadType == 0x41
			&& Header.HeadFlags == 0x00
			&& Header.HeadSize == 0x0008)
	{
		puts("ReadHeader,1,good,pass");
	}
	else {
		puts("ReadHeader,1,good,fail");
		fprintf(stderr, "CRC=%04X,Type=%02X,Flags=%02X,Size=%04X\n",
				Header.HeadCRC,
				Header.HeadType,
				Header.HeadFlags,
				Header.HeadSize);
	}

	return 0;
}
#endif
