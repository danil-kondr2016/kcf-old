#include "errors.h"
#include "kcf_impl.h"
#include "record.h"
#include "bytepack.h"

#include <stdlib.h>
#include <string.h>

KCFERROR RecordToFileHeader(
	struct KcfRecord *Record,
	struct KcfFileHeader *Header
)
{
	ptrdiff_t Offset = 0;
	size_t Size;
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
	Size = Record->DataSize;
	ReadU8(pbuf, Size, &Offset, &Header->FileFlags);
	ReadU8(pbuf, Size, &Offset, &Header->FileType);

	switch (Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) {
	case KCF_FILE_HAS_UNPACKED_8:
		ReadU64LE(pbuf, Size, &Offset, &Header->UnpackedSize);
		break;
	case KCF_FILE_HAS_UNPACKED_4:
		ReadU32LE(pbuf, Size, &Offset, &Header->UnpackedSize);
		break;
	}

	if (Header->FileFlags & KCF_FILE_HAS_FILE_CRC32) {
		ReadU32LE(pbuf, Size, &Offset, &Header->FileCRC32);
	}

	ReadU32LE(pbuf, Size, &Offset, &Header->CompressionInfo);

	if (Header->FileFlags & KCF_FILE_HAS_TIMESTAMP) {
		ReadU64LE(pbuf, Size, &Offset, &Header->TimeStamp);
		pbuf += 8;
	}

	ReadU16LE(pbuf, Size, &Offset, &Header->FileNameSize);
	Header->FileName = malloc(Header->FileNameSize + 1);
	if (!Header->FileName) {
		return KCF_ERROR_OUT_OF_MEMORY;
	}
	Header->FileName[Header->FileNameSize] = 0;
	memcpy(Header->FileName, pbuf+Offset, Header->FileNameSize);

	return KCF_ERROR_OK;
}

static
int get_file_header_data_size(struct KcfFileHeader *Header)
{
	int result;

	result = 2; /* FileFlags, FileType */
	if ((Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) == KCF_FILE_HAS_UNPACKED_8)
		result += 8; /* UnpackedSize */
	else if ((Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) == KCF_FILE_HAS_UNPACKED_4)
		result += 4; /* UnpackedSize */

	if ((Header->FileFlags & KCF_FILE_HAS_FILE_CRC32))
		result += 4; /* FileCRC32 */
	if ((Header->FileFlags & KCF_FILE_HAS_TIMESTAMP))
		result += 8; /* TimeStamp */

	result += 4; /* CompressionInfo */
	result += 2; /* FileNameSize */
	result += Header->FileNameSize;

	return result;
}

KCFERROR FileHeaderToRecord(
	struct KcfFileHeader *Header, 
	struct KcfRecord *Record
)
{
	int size;
	ptrdiff_t offset = 0;

	if (!Record || !Header)
		return KCF_ERROR_INVALID_PARAMETER;

	Record->Header.HeadType = KCF_FILE_HEADER;
	size = get_file_header_data_size(Header);
	Record->Data = malloc(size);
	if (!Record->Data)
		return KCF_ERROR_OUT_OF_MEMORY;
	Record->DataSize = size;

	WriteU8(Record->Data, size, &offset, Header->FileFlags);
	WriteU8(Record->Data, size, &offset, Header->FileType);
	if ((Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) == KCF_FILE_HAS_UNPACKED_8)
		WriteU64LE(Record->Data, size, &offset, Header->UnpackedSize);
	else if ((Header->FileFlags & KCF_FILE_HAS_UNPACKED_8) == KCF_FILE_HAS_UNPACKED_4)
		WriteU32LE(Record->Data, size, &offset, Header->UnpackedSize);
	if ((Header->FileFlags & KCF_FILE_HAS_FILE_CRC32))
		WriteU32LE(Record->Data, size, &offset, Header->FileCRC32);
	WriteU32LE(Record->Data, size, &offset, Header->CompressionInfo);
	if ((Header->FileFlags & KCF_FILE_HAS_TIMESTAMP))
		WriteU64LE(Record->Data, size, &offset, Header->TimeStamp);
	WriteU16LE(Record->Data, size, &offset, Header->FileNameSize);
	memcpy(Record->Data + offset, Header->FileName, Header->FileNameSize);

	FixRecord(Record);
	return KCF_ERROR_OK;
}

void ClearFileHeader(struct KcfFileHeader *Header)
{
	if (Header->FileName)
		free(Header->FileName);

	memset(Header, 0, sizeof(*Header));
}
