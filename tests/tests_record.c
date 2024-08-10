#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../kcf.h"
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
	Record.Header.HeadCRC = 0x0000;
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
