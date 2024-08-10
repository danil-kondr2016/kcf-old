#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../kcf.h"
#include "../record.h"

bool test1(void);
bool test2(void);

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
