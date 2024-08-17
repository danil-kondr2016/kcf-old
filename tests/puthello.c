#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../kcf.h"
#include "../record.h"

static char *OutputPath = "first.kcf";

static char *HelloName = "hello.txt";
static char *HelloData = "Hello, world!\r\n";

static uint8_t ArchiveHeaderData[] = {1, 0};

int main(void)
{
	HKCF hKCF;
	KCFERROR Error;
	struct KcfRecord ArchiveHeader = {0};
	struct KcfRecord FileHeaderRec = {0};
	struct KcfFileHeader FileHeader = {0};
	time_t Time;

	Error = CreateArchive(OutputPath, KCF_MODE_CREATE, &hKCF);
	if (Error) {
		printf("Failed to create archive %s: %s\n", OutputPath, GetKcfErrorString(Error));
		return 1;
	}
	
	Time = time(NULL);

	WriteArchiveMarker(hKCF);
	ArchiveHeader.Header.HeadType = KCF_ARCHIVE_HEADER;
	ArchiveHeader.Data = ArchiveHeaderData;
	ArchiveHeader.DataSize = sizeof(ArchiveHeaderData);
	WriteRecord(hKCF, &ArchiveHeader);

	FileHeader.FileFlags = KCF_FILE_HAS_UNPACKED_4 | KCF_FILE_HAS_TIMESTAMP;
	FileHeader.UnpackedSize = strlen(HelloData);
	FileHeader.FileNameSize = strlen(HelloName);
	FileHeader.TimeStamp = Time;
	FileHeader.FileName = HelloName;
	FileHeaderToRecord(&FileHeader, &FileHeaderRec);

	WriteRecordWithAddedData(hKCF, &FileHeaderRec, HelloData, strlen(HelloData));
	CloseArchive(hKCF);

	return 0;
}
