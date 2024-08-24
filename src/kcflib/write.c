#include <kcf/archive.h>
#include <kcf/errors.h>

#include <stdio.h>
#include <stdlib.h>

#include "crc32c.h"
#include "kcf_impl.h"
#include "stdio64.h"

KCFERROR KCF_write_record(KCF *kcf, struct KcfRecord *Record)
{
	uint8_t *buffer;

	trace_kcf_msg("WriteRecord begin");
	trace_kcf_record(Record);
	trace_kcf_state(kcf);

	if (!kcf->IsWriting)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (kcf->WriterState == KCF_WRSTATE_IDLE)
		kcf->WriterState = KCF_WRSTATE_RECORD;
	if (kcf->WriterState == KCF_WRSTATE_MARKER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (kcf->WriterState == KCF_WRSTATE_ADDED_DATA)
		KCF_finish_added_data(kcf);

	/* Ensure that record CRC32 is valid */
	rec_fix(Record);
	buffer = malloc(Record->Header.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	kcf->RecordOffset = kcf_ftell(kcf->File);
	rec_to_buffer(Record, buffer, Record->Header.HeadSize);
	if (!fwrite(buffer, Record->Header.HeadSize, 1, kcf->File))
		return trace_kcf_error(
		    kcf_file_error(kcf->File, KCF_SITUATION_WRITING));

	/* Save all information for backpatching */
	kcf->HasAddedSize = !!(Record->Header.HeadFlags & KCF_HAS_ADDED_SIZE_4);
	kcf->HasAddedDataCRC32 =
	    !!(Record->Header.HeadFlags & KCF_HAS_ADDED_DATA_CRC32);
	if (kcf->HasAddedSize) {
		rec_copy(&kcf->LastRecord, Record);
		kcf->AddedDataToBeWritten = kcf->LastRecord.Header.AddedSize;
		kcf->WriterState          = KCF_WRSTATE_ADDED_DATA;
	}

	trace_kcf_state(kcf);
	trace_kcf_msg("WriteRecord end");

	return KCF_ERROR_OK;
}

KCFERROR KCF_write_record_with_added_data(KCF *kcf, struct KcfRecord *Record,
                                          uint8_t *AddedData, size_t Size)
{
	uint8_t *buffer;

	trace_kcf_msg("WriteRecordWithAddedData begin");
	trace_kcf_state(kcf);

	if (!kcf->IsWriting)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (kcf->WriterState == KCF_WRSTATE_IDLE)
		kcf->WriterState = KCF_WRSTATE_RECORD;
	if (kcf->WriterState == KCF_WRSTATE_IDLE ||
	    kcf->WriterState == KCF_WRSTATE_MARKER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (kcf->WriterState == KCF_WRSTATE_ADDED_DATA)
		KCF_finish_added_data(kcf);

	/* Ensure that record CRC32 is valid */
	if (AddedData && Size) {
		if (rec_has_added_data_CRC(Record)) {
			Record->Header.AddedDataCRC32 =
			    crc32c(0, AddedData, Size);
		}

		if (Size > 2147483647) {
			Record->Header.HeadFlags |= KCF_HAS_ADDED_SIZE_8;
		} else {
			Record->Header.HeadFlags |= KCF_HAS_ADDED_SIZE_4;
		}
		Record->Header.AddedSize = Size;
	} else {
		Record->Header.HeadFlags &= ~KCF_HAS_ADDED_DATA_CRC32;
		Record->Header.HeadFlags &= ~KCF_HAS_ADDED_SIZE_8;
		Record->Header.AddedSize      = 0;
		Record->Header.AddedDataCRC32 = 0;
	}

	rec_fix(Record);
	buffer = malloc(Record->Header.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	rec_to_buffer(Record, buffer, Record->Header.HeadSize);
	if (!fwrite(buffer, Record->Header.HeadSize, 1, kcf->File))
		return trace_kcf_error(
		    kcf_file_error(kcf->File, KCF_SITUATION_WRITING));
	if (!fwrite(AddedData, Size, 1, kcf->File))
		return trace_kcf_error(
		    kcf_file_error(kcf->File, KCF_SITUATION_WRITING));

	trace_kcf_state(kcf);
	trace_kcf_msg("WriteRecord end");

	return KCF_ERROR_OK;
}

KCFERROR KCF_write_added_data(KCF *kcf, uint8_t *AddedData, size_t Size)
{
	trace_kcf_msg("WriteAddedData begin");
	trace_kcf_state(kcf);

	if (!kcf->IsWriting || kcf->WriterState != KCF_WRSTATE_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (kcf->AddedDataToBeWritten > 0) {
		size_t Remaining;

		Remaining = kcf->AddedDataToBeWritten - kcf->WrittenAddedData;
		if (Size > Remaining)
			Size = Remaining;

		if (Remaining == 0)
			return KCF_ERROR_OK;
	}

	if (!fwrite(AddedData, Size, 1, kcf->File))
		return trace_kcf_error(
		    kcf_file_error(kcf->File, KCF_SITUATION_WRITING));

	kcf->WrittenAddedData += Size;
	if (kcf->HasAddedDataCRC32)
		kcf->AddedDataCRC32 =
		    crc32c(kcf->AddedDataCRC32, AddedData, Size);

	trace_kcf_state(kcf);
	trace_kcf_msg("WriteAddedData end");

	return KCF_ERROR_OK;
}

KCFERROR KCF_finish_added_data(KCF *kcf)
{
	uint8_t *buffer;

	trace_kcf_msg("FinishAddedData begin");
	trace_kcf_state(kcf);

	if (!kcf->IsWriting || kcf->WriterState != KCF_WRSTATE_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	/* If data size is known and no CRC32 of added data is calculated, don't
	 * backpatch */
	if (kcf->AddedDataToBeWritten > 0 && !kcf->HasAddedDataCRC32) {
		goto cleanup;
	}

	kcf->RecordEndOffset = kcf_ftell(kcf->File);
	if (kcf_fseek(kcf->File, kcf->RecordOffset, SEEK_SET) == -1)
		return trace_kcf_error(KCF_ERROR_WRITE);

	kcf->LastRecord.Header.AddedSize      = kcf->WrittenAddedData;
	kcf->LastRecord.Header.AddedDataCRC32 = kcf->AddedDataCRC32;
	rec_fix(&kcf->LastRecord);
	buffer = malloc(kcf->LastRecord.Header.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);
	rec_to_buffer(&kcf->LastRecord, buffer,
	              kcf->LastRecord.Header.HeadSize);

	if (!fwrite(buffer, kcf->LastRecord.Header.HeadSize, 1, kcf->File))
		return trace_kcf_error(
		    kcf_file_error(kcf->File, KCF_SITUATION_WRITING));
	kcf_fseek(kcf->File, kcf->RecordEndOffset, SEEK_SET);

cleanup:
	rec_clear(&kcf->LastRecord);
	kcf->RecordOffset         = 0;
	kcf->RecordEndOffset      = 0;
	kcf->WrittenAddedData     = 0;
	kcf->AddedDataToBeWritten = 0;
	kcf->AddedDataCRC32       = 0;
	kcf->HasAddedDataCRC32    = false;
	kcf->HasAddedSize         = false;
	kcf->WriterState          = KCF_WRSTATE_RECORD;

	trace_kcf_state(kcf);
	trace_kcf_msg("FinishAddedData end");

	return KCF_ERROR_OK;
}

KCFERROR KCF_write_archive_header(KCF *kcf, int Reserved)
{
	KCFERROR Error;
	struct KcfArchiveHeader ahdr;
	struct KcfRecord Record;

	(void)Reserved;
	ahdr.ArchiveVersion = 1;
	rec_from_archive_header(&ahdr, &Record);

	Error = KCF_write_record(kcf, &Record);
	rec_clear(&Record);
	return Error;
}
