#include "unpack.h"
#include "kcf_impl.h"

#include <errno.h>

KCFERROR UnpackCurrentFile(HKCF hKCF)
{
	KCFERROR Error = KCF_ERROR_OK;
	FILE *OutputFile;
	uint8_t Buffer[4096];
	size_t ToRead, BytesRead, BytesWritten, Remaining;

	Error = ReadRecord(hKCF, &hKCF->LastRecord);
	if (Error)
		goto cleanup0;
	
	if (hKCF->LastRecord.Header.HeadType != KCF_FILE_HEADER) {
		Error = KCF_ERROR_INVALID_FORMAT;
		goto cleanup1;
	}

	Error = RecordToFileHeader(&hKCF->LastRecord, &hKCF->CurrentFile);
	if (Error)
		goto cleanup1;
	
	OutputFile = fopen(hKCF->CurrentFile.FileName, "wb");
	if (!OutputFile) {
		Error = ErrnoToKcf();
		goto cleanup2;
	}
	
	/* TODO compression! */
	do {
		Remaining = hKCF->LastRecord.Header.AddedSize;
		ToRead = 4096;
		while (IsAddedDataAvailable(hKCF)) {
		    if (ToRead > Remaining)
		        ToRead = Remaining;

		    Error = ReadAddedData(hKCF, Buffer, ToRead, &BytesRead);
		    if (Error)
		        goto cleanup3;
		    
		    BytesWritten = fwrite(Buffer, 1, BytesRead, OutputFile);
		    if (BytesWritten < BytesRead) {
		        Error = KCF_ERROR_WRITE;
		        goto cleanup3;
		    }
		}

		if (hKCF->LastRecord.Header.HeadFlags & 0x01) {
		    Error = ReadRecord(hKCF, &hKCF->LastRecord);
		    if (Error)
		        goto cleanup3;

		    if (hKCF->LastRecord.Header.HeadType != KCF_DATA_FRAGMENT) {
		        Error = KCF_ERROR_INVALID_FORMAT;
		        goto cleanup3;
		    }
		}
	}
	while (hKCF->LastRecord.Header.HeadFlags & 0x01);

cleanup3:
	fclose(OutputFile);
cleanup2:
	ClearFileHeader(&hKCF->CurrentFile);
cleanup1:
	ClearRecord(&hKCF->LastRecord);
cleanup0:
	return Error;
}