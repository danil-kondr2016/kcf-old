#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"
#include "pack.h"
#include "unpack.h"

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

	argc--; argc--;
	argv++; argv++;

	if (Command[1] == '\0') {
		switch (Command[0]) {
		case 'c':
			return pack(argc, argv);
		case 'x':
			return unpack(argc, argv);
		default:
			return invalid_command(Command);
		}
	}
	else {
	   return invalid_command(Command);
	}

	return 1;
}

static int pack(int argc, char **argv)
{
	char *OutputName, *InputName;
	HKCF Output;
	KCFERROR Error;

	OutputName = argv[0];
	argc--; argv++;

	Error = CreateArchive(OutputName, KCF_MODE_CREATE, &Output);
	if (Error) {
		printf("%s: failed to create archive %s: %s\n", 
				Program, 
				OutputName, 
				GetKcfErrorString(Error));
		return 1;
	}

	printf("Creating archive %s...\n", OutputName);

	WriteArchiveMarker(Output);
	WriteArchiveHeader(Output,  0);

	for (InputName = *argv; InputName; argv++, argc--, InputName = *argv) {
		printf("Packing file %s...\n", InputName);
		Error = PackFile(Output, InputName, 0);
		if (Error) {
			printf("%s: failed to pack file %s: %s\n", 
					Program, 
					InputName, 
					GetKcfErrorString(Error));
			return 1;
		}
	}

	CloseArchive(Output);

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
	argc--; argv++;

	Error = CreateArchive(ArchiveName, KCF_MODE_READ, &Archive);
	if (Error) {
		printf("%s: failed to create archive %s: %s\n", 
				Program, 
				ArchiveName, 
				GetKcfErrorString(Error));
		return 1;
	}

	ScanArchiveForMarker(Archive);
	ReadRecord(Archive, &Record);
	Error = RecordToArchiveHeader(&Record, &Header);
	if (Error) {
		printf("%s: %s: %s\n", 
				Program, 
				ArchiveName, 
				GetKcfErrorString(Error));
		return 1;
	}

	puts("Unpacking files...");
	while (!Error) {
		Error = UnpackCurrentFile(Archive);
	}

	CloseArchive(Archive);

	if (Error) {
		printf("%s: %s: %s\n", 
				Program, 
				ArchiveName, 
				GetKcfErrorString(Error));
		return 1;
	}
	
	return 0;
}
