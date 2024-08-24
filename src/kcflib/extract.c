#include <kcf/archive.h>

#include <assert.h>

#include "kcf_impl.h"

static KCFERROR read_file_info(KCF *kcf, struct KcfFileInfo *FileInfo);

KCFERROR KCF_get_current_file_info(KCF *kcf, struct KcfFileInfo *FileInfo)
{
	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;
	if (!FileInfo)
		return KCF_ERROR_INVALID_PARAMETER;

	switch (kcf->UnpackerState) {
	case KCF_UPSTATE_FILE_HEADER:
		return read_file_info(kcf, FileInfo);
	case KCF_UPSTATE_FILE_DATA:
	case KCF_UPSTATE_AFTER_FILE_DATA:
		return file_info_copy(FileInfo, &kcf->CurrentFile);
	default:
		return KCF_ERROR_INVALID_STATE;
	}
}

KCFERROR KCF_skip_file(KCF *kcf)
{
	struct KcfRecord Record     = {0};
	struct KcfFileInfo FileInfo = {0};
	KCFERROR Error              = KCF_ERROR_OK;

	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;
	if (kcf->IsWriting)
		return KCF_ERROR_INVALID_STATE;

	do {
		Error = KCF_read_record(kcf, &Record);
		if (Error)
			goto cleanup;

		switch (kcf->UnpackerState) {
		case KCF_UPSTATE_FILE_HEADER:
			kcf->UnpackerState = KCF_UPSTATE_FILE_DATA;
			if (rec_has_added_size(&Record))
				KCF_skip_record(kcf);
			break;
		case KCF_UPSTATE_FILE_DATA:
			if (Record.HeadType != KCF_DATA_FRAGMENT) {
				Error = KCF_ERROR_INVALID_DATA;
				goto cleanup;
			}

			if (rec_has_added_size(&Record))
				KCF_skip_record(kcf);
			break;
		default:
			return KCF_ERROR_INVALID_STATE;
		}
	} while (Record.HeadFlags & 0x01);

	kcf->UnpackerState = KCF_UPSTATE_FILE_HEADER;
cleanup:
	file_info_clear(&FileInfo);
	rec_clear(&Record);
	return Error;
}

KCFERROR KCF_extract(KCF *kcf, BIO *Output)
{
	KCFERROR Error = KCF_ERROR_OK;
	uint8_t Buffer[4096];
	size_t ToRead, BytesRead, BytesWritten, Remaining;
	int ret;

	if (!kcf || !Output)
		return KCF_ERROR_INVALID_PARAMETER;
	if (kcf->IsWriting || kcf->UnpackerState != KCF_UPSTATE_FILE_HEADER)
		return KCF_ERROR_INVALID_STATE;

	Error = KCF_read_record(kcf, &kcf->LastRecord);
	if (Error)
		goto cleanup0;

	if (kcf->LastRecord.HeadType != KCF_FILE_HEADER) {
		Error = KCF_ERROR_INVALID_FORMAT;
		goto cleanup1;
	}

	Error = record_to_file_info(&kcf->LastRecord, &kcf->CurrentFile);
	if (Error)
		goto cleanup1;

	/* TODO compression! */
	do {
		Remaining = kcf->LastRecord.AddedSize;
		ToRead    = 4096;
		while (KCF_is_added_data_available(kcf)) {
			if (ToRead > Remaining)
				ToRead = Remaining;

			Error = KCF_read_added_data(kcf, Buffer, ToRead,
			                            &BytesRead);
			if (Error)
				goto cleanup2;

			ret = BIO_write_ex(Output, Buffer, BytesRead,
			                   &BytesWritten);
			if (!ret) {
				Error = KCF_ERROR_WRITE;
				goto cleanup2;
			}
		}

		if (kcf->LastRecord.HeadFlags & 0x01) {
			Error = KCF_read_record(kcf, &kcf->LastRecord);
			if (Error)
				goto cleanup2;

			if (kcf->LastRecord.HeadType != KCF_DATA_FRAGMENT) {
				Error = KCF_ERROR_INVALID_FORMAT;
				goto cleanup2;
			}
		}
	} while (kcf->LastRecord.HeadFlags & 0x01);

cleanup2:
	file_info_clear(&kcf->CurrentFile);
cleanup1:
	rec_clear(&kcf->LastRecord);
cleanup0:
	return Error;
}

static KCFERROR read_file_info(KCF *kcf, struct KcfFileInfo *FileInfo)
{
	struct KcfRecord Record = {0};
	KCFERROR Error          = KCF_ERROR_OK;

	Error = KCF_read_record(kcf, &Record);
	if (Error)
		return Error;

	Error = record_to_file_info(&Record, FileInfo);
	if (Error)
		goto cleanup;

	Error              = file_info_copy(&kcf->CurrentFile, FileInfo);
	kcf->UnpackerState = KCF_UPSTATE_FILE_DATA;

cleanup:
	rec_clear(&Record);
	return Error;
}
