#include "crc32c.h"
#include "errors.h"
#include "record.h"
#include "kcf.h"
#include <stdio.h>
#include <stdlib.h>

#include "kcf_impl.h"

#ifdef __linux__
static inline int64_t _ftelli64(FILE *f)
{
	return ftello(f);
}

static inline int64_t _fseeki64(FILE *f, long long off, int whence)
{
	return fseeko(f, off, whence);
}
#endif

KCFERROR WriteRecord(HKCF hKCF, struct KcfRecord *Record)
{
	uint8_t *buffer;

	trace_kcf_msg("WriteRecord begin");
	trace_kcf_state(hKCF);

	if (!hKCF->IsWriting)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (hKCF->WriterState == KCF_STATE_WRITING_IDLE)
		hKCF->WriterState = KCF_STATE_WRITING_MAIN_RECORD;
	if (hKCF->WriterState == KCF_STATE_WRITING_MARKER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (hKCF->WriterState == KCF_STATE_WRITING_ADDED_DATA)
		FinishAddedData(hKCF);

	/* Ensure that record CRC32 is valid */
	FixRecord(Record);
	buffer = malloc(Record->Header.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	hKCF->RecordOffset = _ftelli64(hKCF->File);
	RecordToBuffer(Record, buffer, Record->Header.HeadSize);
	if (!fwrite(buffer, Record->Header.HeadSize, 1, hKCF->File))
		return trace_kcf_error(FileErrorToKcf(hKCF->File, KCF_SITUATION_WRITING));

	/* Save all information for backpatching */
	hKCF->HasAddedSize = !!(hKCF->LastRecord.Header.HeadFlags & KCF_HAS_ADDED_SIZE_4);
	hKCF->HasAddedDataCRC32 = !!(hKCF->LastRecord.Header.HeadFlags & KCF_HAS_ADDED_DATA_CRC32);
	if (hKCF->HasAddedSize) {
		CopyRecord(&hKCF->LastRecord, Record);
		hKCF->AddedDataToBeWritten = hKCF->LastRecord.Header.AddedSize;
		hKCF->WriterState = KCF_STATE_WRITING_ADDED_DATA;
	}

	trace_kcf_state(hKCF);
	trace_kcf_msg("WriteRecord end");

	return KCF_ERROR_OK;
}

KCFERROR WriteRecordWithAddedData(HKCF hKCF, struct KcfRecord *Record, 
	uint8_t *AddedData, size_t Size
)
{
	uint8_t *buffer;

	trace_kcf_msg("WriteRecordWithAddedData begin");
	trace_kcf_state(hKCF);

	if (!hKCF->IsWriting)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	if (hKCF->WriterState == KCF_STATE_WRITING_IDLE)
		hKCF->WriterState = KCF_STATE_WRITING_MAIN_RECORD;
	if (hKCF->WriterState == KCF_STATE_WRITING_IDLE || hKCF->WriterState == KCF_STATE_WRITING_MARKER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);
	
	if (hKCF->WriterState == KCF_STATE_WRITING_ADDED_DATA)
		FinishAddedData(hKCF);

	/* Ensure that record CRC32 is valid */
	if (AddedData && Size) {
		if (HasAddedDataCRC32(Record)) {
			Record->Header.AddedDataCRC32 = crc32c(0, AddedData, Size);
		}

		if (Size > 2147483647) {
			Record->Header.HeadFlags |= KCF_HAS_ADDED_SIZE_8;
		}
		else {
			Record->Header.HeadFlags |= KCF_HAS_ADDED_SIZE_4;
		}
		Record->Header.AddedSize = Size;
	}
	else {
		Record->Header.HeadFlags &= ~KCF_HAS_ADDED_DATA_CRC32;
		Record->Header.HeadFlags &= ~KCF_HAS_ADDED_SIZE_8;
		Record->Header.AddedSize = 0;
		Record->Header.AddedDataCRC32 = 0;
	}
	
	FixRecord(Record);
	buffer = malloc(Record->Header.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	RecordToBuffer(Record, buffer, Record->Header.HeadSize);
	if (!fwrite(buffer, Record->Header.HeadSize, 1, hKCF->File))
		return trace_kcf_error(FileErrorToKcf(hKCF->File, KCF_SITUATION_WRITING));
	if (!fwrite(AddedData, Size, 1, hKCF->File))
		return trace_kcf_error(FileErrorToKcf(hKCF->File, KCF_SITUATION_WRITING));

	trace_kcf_state(hKCF);
	trace_kcf_msg("WriteRecord end");

	return KCF_ERROR_OK;
}

KCFERROR WriteAddedData(HKCF hKCF, uint8_t *AddedData, size_t Size)
{
	trace_kcf_msg("WriteAddedData begin");
	trace_kcf_state(hKCF);

	if (!hKCF->IsWriting || hKCF->WriterState != KCF_STATE_WRITING_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (hKCF->AddedDataToBeWritten > 0) {
		size_t Remaining;

		Remaining = hKCF->AddedDataToBeWritten - hKCF->WrittenAddedData;
		if (Size > Remaining)
			Size = Remaining;

		if (Remaining == 0)
			return KCF_ERROR_OK;
	}

	if (!fwrite(AddedData, Size, 1, hKCF->File))
		return trace_kcf_error(FileErrorToKcf(hKCF->File, KCF_SITUATION_WRITING));

	hKCF->WrittenAddedData += Size;
	if (hKCF->HasAddedDataCRC32)
		hKCF->AddedDataCRC32 = crc32c(hKCF->AddedDataCRC32, AddedData, Size);

	trace_kcf_state(hKCF);
	trace_kcf_msg("WriteAddedData end");

	return KCF_ERROR_OK;
}

KCFERROR FinishAddedData(HKCF hKCF)
{
	uint8_t *buffer;

	trace_kcf_msg("FinishAddedData begin");
	trace_kcf_state(hKCF);

	if (!hKCF->IsWriting || hKCF->WriterState != KCF_STATE_WRITING_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	/* If data size is known and no CRC32 of added data is calculated, don't backpatch */
	if (hKCF->AddedDataToBeWritten > 0 && hKCF->HasAddedDataCRC32) {
		goto cleanup; 
	}

	hKCF->RecordEndOffset = _ftelli64(hKCF->File);
	if (_fseeki64(hKCF->File, hKCF->RecordOffset, SEEK_SET) == -1)
		return trace_kcf_error(KCF_ERROR_WRITE);

	hKCF->LastRecord.Header.AddedSize = hKCF->WrittenAddedData;
	hKCF->LastRecord.Header.AddedDataCRC32 = hKCF->AddedDataCRC32;
	FixRecord(&hKCF->LastRecord);
	buffer = malloc(hKCF->LastRecord.Header.HeadSize);
	if (!buffer)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	if (!fwrite(buffer, hKCF->LastRecord.Header.HeadSize, 1, hKCF->File))
		return trace_kcf_error(FileErrorToKcf(hKCF->File, KCF_SITUATION_WRITING));
	_fseeki64(hKCF->File, hKCF->RecordEndOffset, SEEK_SET);

cleanup:
	ClearRecord(&hKCF->LastRecord);
	hKCF->RecordOffset = 0;
	hKCF->RecordEndOffset = 0;
	hKCF->WrittenAddedData = 0;
	hKCF->AddedDataToBeWritten = 0;
	hKCF->AddedDataCRC32 = 0;
	hKCF->HasAddedDataCRC32 = false;
	hKCF->HasAddedSize = false;
	hKCF->WriterState = KCF_STATE_WRITING_MAIN_RECORD;

	trace_kcf_state(hKCF);
	trace_kcf_msg("FinishAddedData end");

	return KCF_ERROR_OK;
}