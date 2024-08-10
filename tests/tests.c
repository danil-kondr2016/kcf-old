#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../kcf.h"
#include "../record.h"

HKCF hKCF = NULL;

bool test1_marker_good(void);
bool test2_marker_bad(void);
bool test3_record_header(void);
bool test4_record_header(void);
bool test5_record_header(void);
bool test6_record_header(void);
bool test7_record_header(void);

int main(void)
{
	plan_tests(7);
	ok(test1_marker_good(), "file with valid marker");
	ok(test2_marker_bad(), "file without valid marker");
	ok(test3_record_header(), "read archive header record");
	ok(test4_record_header(), "skip one record and read");
	ok(test5_record_header(), "read record with added size");
	ok(test6_record_header(), "read added data");
	ok(test7_record_header(), "read chunk after added data");

	if (hKCF)
		CloseArchive(hKCF);

	return exit_status();
}

bool test1_marker_good(void)
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

bool test2_marker_bad(void)
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

bool test3_record_header(void)
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

bool test4_record_header(void)
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

bool test5_record_header(void)
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

bool test6_record_header(void)
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

bool test7_record_header(void)
{
	KCFERROR Error;
	struct KcfRecord Record;
	bool result;

	Error = ReadRecord(hKCF, &Record);
	result = Error == KCF_ERROR_OK && Record.Header.HeadType == 0x33;

	ClearRecord(&Record);
	return result;
}
