#include "files.h"

#include "kcf_impl.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static KCFERROR read_file_info(HKCF hKCF, struct KcfFileInfo *FileInfo);

KCFERROR GetCurrentFileInfo(HKCF hKCF, struct KcfFileInfo *FileInfo)
{
	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!FileInfo)
		return KCF_ERROR_INVALID_PARAMETER;

	switch (hKCF->UnpackerState) {
	case KCF_UPSTATE_FILE_HEADER:
		return read_file_info(hKCF, FileInfo);
	case KCF_UPSTATE_FILE_DATA:
	case KCF_UPSTATE_AFTER_FILE_DATA:
		return CopyFileInfo(FileInfo, &hKCF->CurrentFile);
	default:
		return KCF_ERROR_INVALID_STATE;
	}
}

KCFERROR SkipFile(HKCF hKCF)
{
	struct KcfRecord Record     = {0};
	struct KcfFileInfo FileInfo = {0};
	KCFERROR Error              = KCF_ERROR_OK;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;
	if (hKCF->IsWriting)
		return KCF_ERROR_INVALID_STATE;

	do {
		Error = ReadRecord(hKCF, &Record);
		if (Error)
			goto cleanup;

		switch (hKCF->UnpackerState) {
		case KCF_UPSTATE_FILE_HEADER:
			hKCF->UnpackerState = KCF_UPSTATE_FILE_DATA;
			if (HasAddedSize4(&Record) || HasAddedSize8(&Record))
				SkipRecord(hKCF);
			break;
		case KCF_UPSTATE_FILE_DATA:
			if (Record.Header.HeadType != KCF_DATA_FRAGMENT) {
				Error = KCF_ERROR_INVALID_DATA;
				goto cleanup;
			}

			if (HasAddedSize4(&Record) || HasAddedSize8(&Record))
				SkipRecord(hKCF);
			break;
		}
	} while (Record.Header.HeadFlags & 0x01);

	hKCF->UnpackerState = KCF_UPSTATE_FILE_HEADER;
cleanup:
	ClearFileInfo(&FileInfo);
	ClearRecord(&Record);
	return Error;
}

KCFERROR ExtractFileData(HKCF hKCF, BIO *Output)
{
	KCFERROR Error = KCF_ERROR_OK;
	uint8_t Buffer[4096];
	size_t ToRead, BytesRead, BytesWritten, Remaining;
	int ret;

	if (!hKCF || !Output)
		return KCF_ERROR_INVALID_PARAMETER;
	if (hKCF->IsWriting || hKCF->UnpackerState != KCF_UPSTATE_FILE_HEADER)
		return KCF_ERROR_INVALID_STATE;

	Error = ReadRecord(hKCF, &hKCF->LastRecord);
	if (Error)
		goto cleanup0;

	if (hKCF->LastRecord.Header.HeadType != KCF_FILE_HEADER) {
		Error = KCF_ERROR_INVALID_FORMAT;
		goto cleanup1;
	}

	Error = RecordToFileInfo(&hKCF->LastRecord, &hKCF->CurrentFile);
	if (Error)
		goto cleanup1;

	/* TODO compression! */
	do {
		Remaining = hKCF->LastRecord.Header.AddedSize;
		ToRead    = 4096;
		while (IsAddedDataAvailable(hKCF)) {
			if (ToRead > Remaining)
				ToRead = Remaining;

			Error = ReadAddedData(hKCF, Buffer, ToRead, &BytesRead);
			if (Error)
				goto cleanup2;

			ret = BIO_write_ex(Output, Buffer, BytesRead,
			                   &BytesWritten);
			if (!ret) {
				Error = KCF_ERROR_WRITE;
				goto cleanup2;
			}
		}

		if (hKCF->LastRecord.Header.HeadFlags & 0x01) {
			Error = ReadRecord(hKCF, &hKCF->LastRecord);
			if (Error)
				goto cleanup2;

			if (hKCF->LastRecord.Header.HeadType !=
			    KCF_DATA_FRAGMENT) {
				Error = KCF_ERROR_INVALID_FORMAT;
				goto cleanup2;
			}
		}
	} while (hKCF->LastRecord.Header.HeadFlags & 0x01);

cleanup2:
	ClearFileInfo(&hKCF->CurrentFile);
cleanup1:
	ClearRecord(&hKCF->LastRecord);
cleanup0:
	return Error;
}

static KCFERROR read_file_info(HKCF hKCF, struct KcfFileInfo *FileInfo)
{
	struct KcfRecord Record = {0};
	KCFERROR Error          = KCF_ERROR_OK;

	Error = ReadRecord(hKCF, &Record);
	if (Error)
		return Error;

	Error = RecordToFileInfo(&Record, FileInfo);
	if (Error)
		goto cleanup;

	Error               = copy_file_header(&hKCF->CurrentFile, FileInfo);
	hKCF->UnpackerState = KCF_UPSTATE_FILE_DATA;

cleanup:
	ClearRecord(&Record);
	return Error;
}
