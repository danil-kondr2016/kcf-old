#include <kcf/files.h>

#include "kcf_impl.h"

KCFERROR BeginFile(HKCF hKCF, struct KcfFileInfo *FileInfo)
{
	KCFERROR Error = KCF_ERROR_OK;
	struct KcfRecord Record;

	if (!hKCF || !FileInfo)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!hKCF->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (hKCF->PackerState != KCF_PKSTATE_IDLE &&
	    hKCF->PackerState != KCF_PKSTATE_FILE_HEADER)
		return KCF_ERROR_INVALID_STATE;

	ClearFileInfo(&hKCF->CurrentFile);
	CopyFileInfo(&hKCF->CurrentFile, FileInfo);

	FileInfoToRecord(FileInfo, &Record);
	Error = WriteRecord(hKCF, &Record);
	if (Error)
		return Error;

	hKCF->PackerState = KCF_PKSTATE_FILE_DATA;

	return Error;
}

#define INSERT_FILE_BUFFER_SIZE 4096

KCFERROR InsertFileData(HKCF hKCF, BIO *Input)
{
	uint8_t Buffer[INSERT_FILE_BUFFER_SIZE];
	size_t BytesRead, BytesWritten;
	int ret;
	KCFERROR Error;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!hKCF->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (hKCF->PackerState != KCF_PKSTATE_FILE_DATA)
		return KCF_ERROR_INVALID_STATE;

	/* TODO compression */
	do {
		ret = BIO_read_ex(Input, Buffer, INSERT_FILE_BUFFER_SIZE,
		                  &BytesRead);
		if (!ret)
			return KCF_ERROR_WRITE;

		Error = WriteAddedData(hKCF, Buffer, BytesRead);
		if (Error)
			return Error;
	} while (BytesRead > 0);

	hKCF->PackerState = KCF_PKSTATE_AFTER_FILE_DATA;

	return Error;
}

KCFERROR EndFile(HKCF hKCF)
{
	KCFERROR Error = KCF_ERROR_OK;

	if (!hKCF)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!hKCF->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (hKCF->PackerState != KCF_PKSTATE_AFTER_FILE_DATA)
		return KCF_ERROR_INVALID_STATE;

	Error = FinishAddedData(hKCF);
	if (Error)
		return Error;

	hKCF->PackerState = KCF_PKSTATE_FILE_HEADER;
	return Error;
}
