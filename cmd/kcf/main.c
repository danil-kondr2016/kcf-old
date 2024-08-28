#ifdef _FILE_OFFSET_BITS
#undef _FILE_OFFSET_BITS
#endif
#define _FILE_OFFSET_BITS 64
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kcf/archive.h>

char *Program = "KCF";

static int pack(int argc, char **argv);
static int unpack(int argc, char **argv);

static int help(void)
{
	puts("KCF version 0.0.2");
	puts("Created by Danila A. Kondratenko");
	puts("");
	puts("Usage: ");
	printf("  %s cmd archive [input1 input2 ... inputN]\n", Program);
	puts("");
	puts("Commands:");
	puts("    c        adds files into archive");
	puts("    x        extracts archive");
	puts("");

	return 0;
}

static int invalid_command(char *cmd)
{
	printf("%s: %s: invalid command\n", Program, cmd);
	return 1;
}

int main(int argc, char **argv)
{
	char *Command;

	Program = argv[0];

	if (argc < 2) {
		return help();
	}

	Command = argv[1];

	argc--;
	argc--;
	argv++;
	argv++;

	if (Command[1] == '\0') {
		switch (Command[0]) {
		case 'c':
			return pack(argc, argv);
		case 'x':
			return unpack(argc, argv);
		default:
			return invalid_command(Command);
		}
	} else {
		return invalid_command(Command);
	}

	return 1;
}

static KCFERROR pack_file(KCF *archive, char *path)
{
	struct KcfFileInfo info = {0};
	IO *f;
	uint8_t buffer[4096];
	size_t to_read, n_read;
	int64_t file_size;
	KCFERROR Error;

	f = IO_open_cfile(path, "rb");
	if (!f) {
		return KCF_ERROR_FILE_NOT_FOUND;
	}

	info.FileType = KCF_FILE_REGULAR;
	info.FileName = path;
	info.HasUnpackedSize = true;
	if (file_size > 2147483647L)
		info.HasUnpackedSize8 = true;
	info.UnpackedSize = file_size;

	if ((Error = KCF_begin_file(archive, &info)))
		goto cleanup;

	if ((Error = KCF_insert_file_data(archive, f)))
		goto cleanup;

	if ((Error = KCF_end_file(archive)))
		goto cleanup;

cleanup:
	IO_close(f);
	return Error;
}

static int pack(int argc, char **argv)
{
	char *OutputName, *InputName;
	IO *out_file;
	KCF *archive;
	KCFERROR Error;

	OutputName = argv[0];
	argc--;
	argv++;

	out_file = IO_open_cfile(OutputName, "w+b");
	if (!out_file) {
		printf("%s: failed to create archive %s: %s\n", Program,
		       OutputName, kcf_error_string(Error));
		return 1;
	}

	Error = KCF_create(out_file, &archive);
	if (Error) {
		printf("%s: failed to create archive %s: %s\n", Program,
		       OutputName, kcf_error_string(Error));
		return 1;
	}

	printf("Creating archive %s...\n", OutputName);

	KCF_init_archive(archive);

	for (InputName = *argv; InputName; argv++, argc--, InputName = *argv) {
		printf("Packing file %s...\n", InputName);
		Error = pack_file(archive, InputName);
		if (Error) {
			printf("%s: failed to pack file %s: %s\n", Program,
			       InputName, kcf_error_string(Error));
			return 1;
		}
	}

	KCF_close(archive);

	return 0;
}

static int unpack(int argc, char **argv)
{
	char *ArchiveName, *OutputName;
	HKCF Archive;
	KCFERROR Error;
	struct KcfArchiveHeader Header;
	struct KcfRecord Record;

	ArchiveName = *argv;
	argc--;
	argv++;

	Error = CreateArchive(ArchiveName, KCF_MODE_READ, &Archive);
	if (Error) {
		printf("%s: failed to create archive %s: %s\n", Program,
		       ArchiveName, GetKcfErrorString(Error));
		return 1;
	}

	ScanArchiveForMarker(Archive);
	ReadRecord(Archive, &Record);
	Error = RecordToArchiveHeader(&Record, &Header);
	if (Error) {
		printf("%s: %s: %s\n", Program, ArchiveName,
		       GetKcfErrorString(Error));
		return 1;
	}

	puts("Unpacking files...");
	while (!Error) {
		Error = UnpackCurrentFile(Archive);
	}

	CloseArchive(Archive);

	if (Error) {
		printf("%s: %s: %s\n", Program, ArchiveName,
		       GetKcfErrorString(Error));
		return 1;
	}

	return 0;
}
