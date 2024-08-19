#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../archive.h"
#include "../record.h"

bool test8(void);
bool test9(void);
bool test10(void);

bool test8(void)
{
	struct KcfRecord Record = {0};
	struct KcfArchiveHeader Header = {0};
	uint8_t Data[2] = {1, 0};
	KCFERROR Error;

	/* Mocking data */
	Record.Header.HeadCRC = 0xB7E9;
	Record.Header.HeadType = KCF_ARCHIVE_HEADER;
	Record.Header.HeadFlags = 0x00;
	Record.Header.HeadSize = 8;
	Record.Data = Data;
	Record.DataSize = 2;

	Error = RecordToArchiveHeader(&Record, &Header);
	if (Error != KCF_ERROR_OK) {
		diag("Failed to read KCF archive header: Error #%d", Error);
		return false;
	}

	if (Header.ArchiveVersion != 1) {
		diag("Invalid archive version, read %d, should be 1",
				Header.ArchiveVersion);
		return false;
	}

	return true;
}

bool test9(void)
{
	struct KcfRecord Record = {0};
	struct KcfArchiveHeader Header = {0};
	uint8_t Data[] = {1, 0};
	KCFERROR Error;

	/* Mocking data */
	Record.Header.HeadCRC = 0x0000;
	Record.Header.HeadType = 0x30;
	Record.Header.HeadFlags = 0x00;
	Record.Header.HeadSize = 8;
	Record.Data = Data;
	Record.DataSize = 2;

	Error = RecordToArchiveHeader(&Record, &Header);
	if (Error != KCF_ERROR_INVALID_DATA) {
		diag("Invalid header has been misrecognized as valid");
		return false;
	}

	return true;
}

bool test10(void)
{
	struct KcfRecord Record = {0};
	struct KcfArchiveHeader Header = {0};
	KCFERROR Error;

	/* Mocking data */
	Record.Header.HeadCRC = 0x0000;
	Record.Header.HeadType = KCF_ARCHIVE_HEADER;
	Record.Header.HeadFlags = 0x00;
	Record.Header.HeadSize = 6;

	Error = RecordToArchiveHeader(&Record, &Header);
	if (Error != KCF_ERROR_INVALID_DATA) {
		diag("Invalid header has been misrecognized as valid");
		return false;
	}

	return true;
}

static const uint8_t test12_data[] = {
	0x04, 'F',
	0x0F, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x09, 0x00,
	'h',  'e',  'l',  'l',  'o',  '.',  't',  'x',  't',
};

bool test12(void)
{
	struct KcfRecord Record = {0};
	struct KcfFileHeader Header = {0};
	KCFERROR Error;

	/* Mocking data */
	Record.Header.HeadCRC = 0x3E39;
	Record.Header.HeadType = KCF_FILE_HEADER;
	Record.Header.HeadFlags = 0x80;
	Record.Header.HeadSize = sizeof(test12_data) + 10;
	Record.Header.AddedSize = 15;
	Record.Data = test12_data;
	Record.DataSize = sizeof(test12_data);

	Error = RecordToFileHeader(&Record, &Header);
	if (Error != KCF_ERROR_OK) {
		diag("Failed to read KCF file header: Error #%d", Error);
		return false;
	}
	
	diag("FileFlags=%02X,UnpackedSize=%lld,FileType=%02X",
		Header.FileFlags, Header.UnpackedSize, Header.FileType);
	diag("CompressionInfo=%08X,FileNameSize=%d,FileName=%s",
		Header.CompressionInfo, Header.FileNameSize, Header.FileName);

	if (Header.FileFlags != 0x04)
		return false;
	if (Header.UnpackedSize != 15)
		return false;
	if (Header.FileType != KCF_REGULAR_FILE)
		return false;
	if (Header.CompressionInfo != 0)
		return false;
	if (Header.FileNameSize != 9)
		return false;
	if (strcmp(Header.FileName, "hello.txt") != 0)
		return false;

	return true;
}
