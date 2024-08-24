#include <kcf/archive.h>

#include "kcf_impl.h"

KCFERROR KCF_begin_file(KCF *kcf, struct KcfFileInfo *FileInfo)
{
	KCFERROR Error = KCF_ERROR_OK;
	struct KcfRecord Record;

	if (!kcf || !FileInfo)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!kcf->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (kcf->PackerState != KCF_PKSTATE_IDLE &&
	    kcf->PackerState != KCF_PKSTATE_FILE_HEADER)
		return KCF_ERROR_INVALID_STATE;

	file_info_clear(&kcf->CurrentFile);
	file_info_copy(&kcf->CurrentFile, FileInfo);

	file_info_to_record(FileInfo, &Record);
	Error = KCF_write_record(kcf, &Record);
	if (Error)
		return Error;

	kcf->PackerState = KCF_PKSTATE_FILE_DATA;

	return Error;
}

#define INSERT_FILE_BUFFER_SIZE 4096

KCFERROR KCF_insert_file_data(KCF *kcf, BIO *Input)
{
	uint8_t Buffer[INSERT_FILE_BUFFER_SIZE];
	size_t BytesRead, BytesWritten;
	int ret;
	KCFERROR Error;

	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!kcf->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (kcf->PackerState != KCF_PKSTATE_FILE_DATA)
		return KCF_ERROR_INVALID_STATE;

	/* TODO compression */
	do {
		ret = BIO_read_ex(Input, Buffer, INSERT_FILE_BUFFER_SIZE,
		                  &BytesRead);
		if (!ret)
			return KCF_ERROR_WRITE;

		Error = KCF_write_added_data(kcf, Buffer, BytesRead);
		if (Error)
			return Error;
	} while (BytesRead > 0);

	kcf->PackerState = KCF_PKSTATE_AFTER_FILE_DATA;

	return Error;
}

KCFERROR KCF_end_file(KCF *kcf)
{
	KCFERROR Error = KCF_ERROR_OK;

	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!kcf->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (kcf->PackerState != KCF_PKSTATE_AFTER_FILE_DATA)
		return KCF_ERROR_INVALID_STATE;

	Error = KCF_finish_added_data(kcf);
	if (Error)
		return Error;

	kcf->PackerState = KCF_PKSTATE_FILE_HEADER;
	return Error;
}
