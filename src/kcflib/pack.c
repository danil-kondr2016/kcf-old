#define _CRT_SECURE_NO_WARNINGS
#include "stdio64.h"
#include "errors.h"
#include "archive.h"
#include "kcf_impl.h"
#include "record.h"
#include "pack.h"

#include <string.h>
#if !defined(_WIN32) || !defined(_WIN64)
#define _strdup strdup
#endif

static inline
int64_t get_file_size(FILE *f)
{
	int64_t old_pos, size;

	old_pos = kcf_ftell(f);
	if (old_pos == -1)
		return -1;

	if (kcf_fseek(f, 0, SEEK_END) == -1)
		return -1;

	size = kcf_ftell(f);
	if (size == -1)
		return -1;

	if (kcf_fseek(f, old_pos, SEEK_SET) == -1)
		return -1;

	return size;
}

KCFERROR PackFile(HKCF hKCF, char *FileName, int PackingMode)
{
	struct KcfFileHeader FileHeader = {0};
	struct KcfRecord Record = {0};
	FILE *File;
	int PotentialFlags = 0;
	uint8_t Buffer[4096];
	size_t ToRead, BytesRead, BytesWritten, Remaining;
	KCFERROR Error = KCF_ERROR_OK;

	/* TODO use PackingMode for specifying packing parameters */
	(void)PackingMode;

	File = fopen(FileName, "rb");
	if (!File) {
		return KCF_ERROR_FILE_NOT_FOUND;
	}

	FileHeader.FileType = KCF_REGULAR_FILE;
	FileHeader.FileNameSize = strlen(FileName);
	FileHeader.FileName = _strdup(FileName);
	FileHeader.UnpackedSize = get_file_size(File);
	FileHeader.FileFlags = 0;
	if (FileHeader.UnpackedSize > 2147483647) {
		FileHeader.FileFlags |= KCF_FILE_HAS_UNPACKED_8;
		PotentialFlags |= KCF_HAS_ADDED_SIZE_8;
	}
	else {
		FileHeader.FileFlags |= KCF_FILE_HAS_UNPACKED_4;
		PotentialFlags |= KCF_HAS_ADDED_SIZE_4;
	}

	FileHeaderToRecord(&FileHeader, &Record);
	Record.Header.HeadFlags = PotentialFlags;
	Record.Header.AddedSize = FileHeader.UnpackedSize;
	FixRecord(&Record);
	WriteRecord(hKCF, &Record);

	Remaining = FileHeader.UnpackedSize;
	ToRead = 4096;
	while (Remaining > 0) {
		if (ToRead > Remaining)
			ToRead = Remaining;

		BytesRead = fread(Buffer, 1, ToRead, File);
		if (BytesRead < 4096 && ferror(File)) {
			Error = KCF_ERROR_READ;
			goto cleanup;
		}

		Error = WriteAddedData(hKCF, Buffer, BytesRead);
		if (Error)
			goto cleanup;

		Remaining -= BytesRead;
	}

cleanup:
	FinishAddedData(hKCF);
	fclose(File);
	ClearRecord(&Record);
	ClearFileHeader(&FileHeader);
	return Error;
}

#ifdef _KCF_PACK_TEST
char *Program;
char *OutputName;
int main(int argc, char **argv)
{
	HKCF Output;
	char *InputName;
	KCFERROR Error;

	if (argc < 2) {
		printf("Usage: %s output [input1 input2 ... inputN]\n", argv[0]);
		return 1;
	}

	Program = *argv;
	argv++; argc--;

	OutputName = *argv;
	argv++; argc--;

	Error = CreateArchive(OutputName, KCF_MODE_CREATE, &Output);
	if (Error) {
		printf("%s: %s\n", Program, GetKcfErrorString(Error));
		return 1;
	}

	WriteArchiveMarker(Output);
	WriteArchiveHeader(Output,  0);

	for (InputName = *argv; InputName; argv++, argc--, InputName = *argv) {
		printf("Packing file %s...\n", InputName);
		Error = PackFile(Output, InputName, 0);
		if (Error) {
			printf("%s: %s\n", Program, GetKcfErrorString(Error));
			return 1;
		}
	}

	return 0;
}

#endif
