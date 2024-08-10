#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../kcf.h"
#include "../record.h"

bool test11(void)
{
	HKCF hKCF;
	KCFERROR Error;
	struct KcfRecord Record;
	bool result = true;

	Error = CreateArchive("test0001.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		diag("Failed to open file test0001.kcf: Error #%d", Error);	
		return false;
	}

	ScanArchiveForMarker(hKCF);

	Error = ReadRecord(hKCF, &Record);
	if (Error) {
		diag("Failed to read record: Error #%d", Error);	
		return false;
	}

	result = ValidateRecord(&Record);

	CloseArchive(hKCF);
	ClearRecord(&Record);

	return result;
}

