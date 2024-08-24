#include <kcf/errors.h>
#include <kcf/files.h>
#include <kcf/record.h>

#include <stdlib.h>
#include <string.h>

#include "bytepack.h"

#define KCF_FILE_HAS_TIMESTAMP  0x01
#define KCF_FILE_HAS_FILE_CRC32 0x02
#define KCF_FILE_HAS_UNPACKED_4 0x04
#define KCF_FILE_HAS_UNPACKED_8 0x0C

static int get_file_info_data_size(struct KcfFileInfo *Info)
{
	int result;
	int file_name_size = 0;

	if (Info->FileName)
		file_name_size = strlen(Info->FileName);

	result = 2; /* FileFlags, FileType */
	if (Info->HasUnpackedSize && Info->HasUnpackedSize8)
		result += 8; /* UnpackedSize */
	else if (Info->HasUnpackedSize && !Info->HasUnpackedSize8)
		result += 4; /* UnpackedSize */

	if (Info->HasFileCRC32)
		result += 4; /* FileCRC32 */
	if (Info->HasTimeStamp)
		result += 8; /* TimeStamp */

	result += 4; /* CompressionInfo */
	result += 2; /* FileNameSize */
	result += file_name_size;

	return result;
}

/* In process of replacement KcfFileHeader with KcfFileInfo,
 * a small potential bug has been fixed. This bug could be activated
 * if the KCF file header record has TimeStamp field.
 *
 * If KCF file header record has TimeStamp field, the TimeStamp
 * will be read into its respective variable and pointer to buffer
 * will be shifted forward to 8 bytes. This can lead to misrepresent
 * FileNameSize field and mangling FileName with some kind of
 * non-allocated heap data. This bug can be potentially used for
 * compromising the system.
 */
KCFERROR
file_info_to_record(struct KcfFileInfo *Info, struct KcfRecord *Record)
{
	ptrdiff_t Offset = 0;
	size_t Size;
	uint8_t *pbuf;
	uint32_t tmp4;
	uint8_t flags, type;
	uint16_t file_name_size;

	if (!Record || !Info)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!rec_validate(Record))
		return KCF_ERROR_INVALID_DATA;
	if (Record->Header.HeadType != KCF_FILE_HEADER)
		return KCF_ERROR_INVALID_DATA;

	pbuf = Record->Data;
	Size = Record->DataSize;
	ReadU8(pbuf, Size, &Offset, &flags);
	ReadU8(pbuf, Size, &Offset, &type);

	Info->HasTimeStamp    = !!(flags & KCF_FILE_HAS_TIMESTAMP);
	Info->HasFileCRC32    = !!(flags & KCF_FILE_HAS_FILE_CRC32);
	Info->HasUnpackedSize = !!(flags & KCF_FILE_HAS_UNPACKED_4);
	Info->HasUnpackedSize8 =
	    (flags & KCF_FILE_HAS_UNPACKED_8) == KCF_FILE_HAS_UNPACKED_8;

	switch (flags & KCF_FILE_HAS_UNPACKED_8) {
	case KCF_FILE_HAS_UNPACKED_8:
		ReadU64LE(pbuf, Size, &Offset, &Info->UnpackedSize);
		break;
	case KCF_FILE_HAS_UNPACKED_4:
		ReadU32LE(pbuf, Size, &Offset, &tmp4);
		Info->UnpackedSize = tmp4;
		break;
	}

	if (Info->HasFileCRC32) {
		ReadU32LE(pbuf, Size, &Offset, &Info->FileCRC32);
	}

	ReadU32LE(pbuf, Size, &Offset, &Info->CompressionInfo);

	if (Info->HasTimeStamp) {
		ReadU64LE(pbuf, Size, &Offset, &Info->TimeStamp);
	}

	ReadU16LE(pbuf, Size, &Offset, &file_name_size);
	Info->FileName = malloc(file_name_size + 1);
	if (!Info->FileName) {
		return KCF_ERROR_OUT_OF_MEMORY;
	}
	Info->FileName[file_name_size] = 0;
	memcpy(Info->FileName, pbuf + Offset, file_name_size);

	return KCF_ERROR_OK;
}

KCFERROR
record_to_file_info(struct KcfRecord *Record, struct KcfFileInfo *Info)
{
	int size;
	ptrdiff_t offset = 0;
	uint8_t file_flags;
	int file_name_size;

	if (!Record || !Info)
		return KCF_ERROR_INVALID_PARAMETER;

	Record->Header.HeadType = KCF_FILE_HEADER;
	size                    = get_file_info_data_size(Info);
	Record->Data            = malloc(size);
	if (!Record->Data)
		return KCF_ERROR_OUT_OF_MEMORY;
	Record->DataSize = size;

	file_flags = 0;
	if (Info->HasTimeStamp)
		file_flags |= KCF_FILE_HAS_TIMESTAMP;
	if (Info->HasFileCRC32)
		file_flags |= KCF_FILE_HAS_FILE_CRC32;
	if (Info->HasUnpackedSize)
		file_flags |= KCF_FILE_HAS_UNPACKED_4;
	if (Info->HasUnpackedSize8)
		file_flags |= KCF_FILE_HAS_UNPACKED_8;

	WriteU8(Record->Data, size, &offset, file_flags);
	WriteU8(Record->Data, size, &offset, Info->FileType);
	if (Info->HasUnpackedSize && Info->HasUnpackedSize8)
		WriteU64LE(Record->Data, size, &offset, Info->UnpackedSize);
	else if (Info->HasUnpackedSize)
		WriteU32LE(Record->Data, size, &offset, Info->UnpackedSize);
	if (Info->HasFileCRC32)
		WriteU32LE(Record->Data, size, &offset, Info->FileCRC32);
	WriteU32LE(Record->Data, size, &offset, Info->CompressionInfo);
	if (Info->HasTimeStamp)
		WriteU64LE(Record->Data, size, &offset, Info->TimeStamp);
	file_name_size = strlen(Info->FileName);
	WriteU16LE(Record->Data, size, &offset, file_name_size);
	memcpy(Record->Data + offset, Info->FileName, file_name_size);

	rec_fix(Record);
	return KCF_ERROR_OK;
}

void file_info_clear(struct KcfFileInfo *Info)
{
	if (Info->FileName)
		free(Info->FileName);

	memset(Info, 0, sizeof(*Info));
}

bool file_info_copy(struct KcfFileInfo *Dest, struct KcfFileInfo *Src)
{
	if (!Dest || !Src)
		return false;

	Dest->TimeStamp    = Src->TimeStamp;
	Dest->UnpackedSize = Src->UnpackedSize;

	if (Src->FileName) {
		int file_name_size = strlen(Src->FileName);
		Dest->FileName     = malloc(file_name_size + 1);
		if (!Dest->FileName)
			return false;
		memcpy(Dest->FileName, Src->FileName, file_name_size);
		Dest->FileName[file_name_size] = 0;
	}

	Dest->FileCRC32        = Src->FileCRC32;
	Dest->CompressionInfo  = Src->CompressionInfo;
	Dest->FileType         = Src->FileType;
	Dest->HasTimeStamp     = Src->HasTimeStamp;
	Dest->HasFileCRC32     = Src->HasFileCRC32;
	Dest->HasUnpackedSize  = Src->HasUnpackedSize;
	Dest->HasUnpackedSize8 = Src->HasUnpackedSize8;

	return true;
}
