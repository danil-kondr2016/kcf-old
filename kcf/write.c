#include <kcf/archive.h>
#include <kcf/errors.h>

#include <stdlib.h>

#include "crc32c.h"
#include "kcf_impl.h"

KCFERROR KCF_write_record(KCF *kcf, struct KcfRecord *Record)
{
	uint8_t *buffer;

	trace_kcf_msg("WriteRecord begin");
	trace_kcf_record(Record);
	trace_kcf_state(kcf);

	if (!KCF_PSTATE_IS_WRITING(kcf->ParserState))
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (kcf->ParserState == KCF_PSTATE_WRITING)
		kcf->ParserState = KCF_PSTATE_WRITE_RECORD;
	if (kcf->ParserState == KCF_PSTATE_WRITE_MARKER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (kcf->ParserState == KCF_PSTATE_WRITE_ADDED_DATA)
		KCF_finish_added_data(kcf);

	/* Ensure that record CRC32 is valid */
	rec_fix(Record);
	buffer = malloc(Record->HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	kcf->RecordOffset = IO_tell(kcf->Stream);
	rec_to_buffer(Record, buffer, Record->HeadSize);
	if (IO_write(kcf->Stream, buffer, Record->HeadSize) < 0)
		return KCF_ERROR_WRITE;

	/* Save all information for backpatching */
	kcf->HasAddedSize = !!(Record->HeadFlags & KCF_HAS_ADDED_SIZE_4);
	kcf->HasAddedDataCRC32 =
	    !!(Record->HeadFlags & KCF_HAS_ADDED_DATA_CRC32);
	if (kcf->HasAddedSize) {
		rec_copy(&kcf->LastRecord, Record);
		kcf->AddedDataToBeWritten = kcf->LastRecord.AddedSize;
		kcf->ParserState          = KCF_PSTATE_WRITE_ADDED_DATA;
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

	if (!KCF_PSTATE_IS_WRITING(kcf->ParserState))
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (kcf->ParserState == KCF_PSTATE_WRITING)
		kcf->ParserState = KCF_PSTATE_WRITE_RECORD;
	if (kcf->ParserState == KCF_PSTATE_WRITE_MARKER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (kcf->ParserState == KCF_PSTATE_WRITE_ADDED_DATA)
		KCF_finish_added_data(kcf);

	/* Ensure that record CRC32 is valid */
	if (AddedData && Size) {
		if (rec_has_added_data_CRC(Record)) {
			Record->AddedDataCRC32 = crc32c(0, AddedData, Size);
		}

		if (Size > 2147483647L) {
			Record->HeadFlags |= KCF_HAS_ADDED_SIZE_8;
		} else {
			Record->HeadFlags |= KCF_HAS_ADDED_SIZE_4;
		}
		Record->AddedSize = Size;
	} else {
		Record->HeadFlags &= ~KCF_HAS_ADDED_DATA_CRC32;
		Record->HeadFlags &= ~KCF_HAS_ADDED_SIZE_8;
		Record->AddedSize      = 0;
		Record->AddedDataCRC32 = 0;
	}

	rec_fix(Record);
	buffer = malloc(Record->HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	rec_to_buffer(Record, buffer, Record->HeadSize);
	if (IO_write(kcf->Stream, buffer, Record->HeadSize) < 0)
		return KCF_ERROR_WRITE;
	if (IO_write(kcf->Stream, AddedData, Size) < 0)
		return KCF_ERROR_WRITE;

	trace_kcf_state(kcf);
	trace_kcf_msg("WriteRecord end");

	return KCF_ERROR_OK;
}

KCFERROR KCF_write_added_data(KCF *kcf, uint8_t *AddedData, size_t Size)
{
	trace_kcf_msg("WriteAddedData begin");
	trace_kcf_state(kcf);

	if (!KCF_PSTATE_IS_WRITING(kcf->ParserState))
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (kcf->ParserState != KCF_PSTATE_WRITE_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (kcf->AddedDataToBeWritten > 0) {
		size_t Remaining;

		Remaining = kcf->AddedDataToBeWritten - kcf->WrittenAddedData;
		if (Size > Remaining)
			Size = Remaining;

		if (Remaining == 0)
			return KCF_ERROR_OK;
	}

	if (IO_write(kcf->Stream, AddedData, Size) < 0)
		return KCF_ERROR_WRITE;

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

	if (!KCF_PSTATE_IS_WRITING(kcf->ParserState))
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (kcf->ParserState != KCF_PSTATE_WRITE_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	/* If data size is known and no CRC32 of added data is calculated, don't
	 * backpatch */
	if (kcf->AddedDataToBeWritten > 0 && !kcf->HasAddedDataCRC32) {
		goto cleanup;
	}

	kcf->RecordEndOffset = IO_tell(kcf->Stream);
	if (IO_seek(kcf->Stream, kcf->RecordOffset, IO_SEEK_SET) < 0)
		return trace_kcf_error(KCF_ERROR_WRITE);

	kcf->LastRecord.AddedSize      = kcf->WrittenAddedData;
	kcf->LastRecord.AddedDataCRC32 = kcf->AddedDataCRC32;
	rec_fix(&kcf->LastRecord);
	buffer = malloc(kcf->LastRecord.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);
	rec_to_buffer(&kcf->LastRecord, buffer, kcf->LastRecord.HeadSize);

	if (IO_write(kcf->Stream, buffer, kcf->LastRecord.HeadSize) < 0)
		return KCF_ERROR_WRITE;
	IO_seek(kcf->Stream, kcf->RecordEndOffset, IO_SEEK_SET);

cleanup:
	rec_clear(&kcf->LastRecord);
	kcf->RecordOffset         = 0;
	kcf->RecordEndOffset      = 0;
	kcf->WrittenAddedData     = 0;
	kcf->AddedDataToBeWritten = 0;
	kcf->AddedDataCRC32       = 0;
	kcf->HasAddedDataCRC32    = false;
	kcf->HasAddedSize         = false;
	kcf->ParserState          = KCF_PSTATE_WRITE_RECORD;

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
