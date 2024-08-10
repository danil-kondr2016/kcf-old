#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../kcf.h"
#include "../record.h"

HKCF hKCF = NULL;

bool test1(void);
bool test2(void);
bool test3(void);
bool test4(void);
bool test5(void);
bool test6(void);
bool test7(void);
bool test8(void);
bool test9(void);
bool test10(void);

int main(void)
{
	plan_tests(10);
	ok(test1(), "file with valid marker");
	ok(test2(), "file without valid marker");
	ok(test3(), "read archive header record");
	ok(test4(), "skip one record and read");
	ok(test5(), "read record with added size");
	ok(test6(), "read added data");
	ok(test7(), "read chunk after added data");
	ok(test8(), "record to archive header (valid)");
	ok(test9(), "record to archive header (invalid by type)");
	ok(test10(), "record to archive header (invalid by size)");

	if (hKCF)
		CloseArchive(hKCF);

	return exit_status();
}

bool test1(void)
{
	HKCF hKCF;
	KCFERROR Error;
	bool result = true;

	Error = CreateArchive("test0001.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		diag("Failed to open file test0001.kcf: Error #%d", Error);
		return false;
	}

	Error = ScanArchiveForMarker(hKCF);
	if (Error != KCF_ERROR_OK) {
		diag("Failed to read archive marker in valid file");
		result = false;
		goto cleanup;
	}

cleanup:
	CloseArchive(hKCF);
	return result;
}

bool test2(void)
{
	HKCF hKCF;
	KCFERROR Error;
	bool result = true;

	Error = CreateArchive("test0002.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		diag("Error #%d", Error);
		return false;
	}

	Error = ScanArchiveForMarker(hKCF);
	if (Error != KCF_ERROR_INVALID_FORMAT) {
		diag("ScanArchiveForMarker returns bad value");
		result = false;
		goto cleanup;
	}

cleanup:
	CloseArchive(hKCF);
	return result;
}

bool test3(void)
{
	HKCF hKCF;
	KCFERROR Error;
	struct KcfRecord Record;

	Error = CreateArchive("test0001.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		diag("Failed to open file test0001.kcf: Error #%d", Error);	
		return false;
	}

	ScanArchiveForMarker(hKCF);

	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		diag("Failed to read record: Error #%d", Error);	
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
		// good;
	}
	else {
		diag("CRC=%04X,Type=%02X,Flags=%02X,Size=%04X",
				Record.Header.HeadCRC,
				Record.Header.HeadType,
				Record.Header.HeadFlags,
				Record.Header.HeadSize);
		return false;
	}
	CloseArchive(hKCF);
	free(Record.Data);

	return true;
}

bool test4(void)
{
	KCFERROR Error;
	struct KcfRecord Record;

	Error = CreateArchive("test0004.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		diag("Failed to open file test0004.kcf: Error #%d", Error);	
		return false;
	}

	ScanArchiveForMarker(hKCF);

	SkipRecord(hKCF);

	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		diag("Failed to read record: Error #%d", Error);
		return true;
	}

	if (Record.Header.HeadCRC == 0x0000
			&& Record.Header.HeadType == 0x30
			&& Record.Header.HeadFlags == 0x00
			&& Record.Header.HeadSize == 0x0010
			&& Record.DataSize == 10)
	{
		// passed;
	}
	else {
		return false;
	}

	return true;
}

bool test5(void)
{
	KCFERROR Error;
	struct KcfRecord Record;
	bool result = true;

	SkipRecord(hKCF);
	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		return false;
	}

	if (Record.Header.HeadType == 0x32 && Record.Header.AddedSize == 10) {
		result = true;
	}
	else {
		result = false;
	}

	ClearRecord(&Record);
	return result;
}

bool test6(void)
{
	uint8_t buf1[12];
	size_t n_read;
	KCFERROR Error;

	Error = ReadAddedData(hKCF, buf1, 12, &n_read);
	if (Error) {
		return false;
	}
	else if (n_read == 10 && memcmp(buf1, "0123456789", 10) == 0) {
		return true;
	}

	return false;
}

bool test7(void)
{
	KCFERROR Error;
	struct KcfRecord Record;
	bool result;

	Error = ReadRecord(hKCF, &Record);
	result = Error == KCF_ERROR_OK && Record.Header.HeadType == 0x33;

	ClearRecord(&Record);
	return result;
}

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


