#include <kcf/archive.h>
#include <kcf/errors.h>

#include <assert.h>
#include <stdlib.h>

#include "bytepack.h"
#include "crc32c.h"
#include "kcf_impl.h"
#include "read.h"
#include "record.h"

/*
 * During the transition from stdio to OpenSSL's BIO, I have fixed
 * another potential bug when reading 8-bit added size field (instead
 * of 4-bit one field)
 */
static KCFERROR read_record_header(KCF *kcf, struct KcfRecord *Record,
                                   size_t *HeaderSize)
{
	uint8_t buffer[18] = {0};
	ptrdiff_t hdr_size = 0;
	size_t n_read;
	int ret;

	assert(kcf);
	assert(Record);

	trace_kcf_msg("read_record_header begin");
	trace_kcf_state(kcf);

	if (!BIO_read_ex(kcf->Stream, buffer, 6, &n_read)) {
		return trace_kcf_error(KCF_ERROR_READ);
	}
	
	ReadU16LE(buffer, 6, &hdr_size, &Record->HeadCRC);
	ReadU8(buffer, 6, &hdr_size, &Record->HeadType);
	ReadU8(buffer, 6, &hdr_size, &Record->HeadFlags);
	ReadU16LE(buffer, 6, &hdr_size, &Record->HeadSize);

	trace_kcf_msg("read_record_header %04X %02X %02X %04X", Record->HeadCRC,
	              Record->HeadType, Record->HeadFlags, Record->HeadSize);

	Record->AddedSize = 0;

	if ((Record->HeadFlags & KCF_HAS_ADDED_SIZE_4) != 0) {
		if (!BIO_read_ex(kcf->Stream, buffer + hdr_size, 4, &n_read))
			return trace_kcf_error(KCF_ERROR_READ);
	}

	if ((Record->HeadFlags & KCF_HAS_ADDED_SIZE_8) ==
	    KCF_HAS_ADDED_SIZE_8) {
		if (!BIO_read_ex(kcf->Stream, buffer + hdr_size + 4, 4, &n_read))
			return trace_kcf_error(KCF_ERROR_READ);
		ReadU64LE(buffer, 14, &hdr_size, &Record->AddedSize);
	} else if ((Record->HeadFlags & KCF_HAS_ADDED_SIZE_8) ==
	           KCF_HAS_ADDED_SIZE_4) {
		ReadU32LE(buffer, 14, &hdr_size, &Record->AddedSizeLow);
	}

	if ((Record->HeadFlags & KCF_HAS_ADDED_DATA_CRC32)) {
		if (!BIO_read_ex(kcf->Stream, buffer + hdr_size, 4, &n_read))
			return trace_kcf_error(KCF_ERROR_READ);
		ReadU32LE(buffer, 18, &hdr_size, &Record->AddedDataCRC32);
		kcf->AddedDataCRC32 = Record->AddedDataCRC32;
	} else {
		kcf->AddedDataCRC32    = 0;
		Record->AddedDataCRC32 = 0;
	}

	trace_kcf_msg("read_record_header %016llX %08X", Record->AddedSize,
	              Record->AddedDataCRC32);

	if (HeaderSize)
		*HeaderSize = hdr_size;
	kcf->ReaderState = KCF_RDSTATE_RECORD_DATA;
	trace_kcf_state(kcf);
	trace_kcf_msg("read_record_header end");
	return KCF_ERROR_OK;
}

KCFERROR KCF_read_record(KCF *kcf, struct KcfRecord *Record)
{
	KCFERROR Error;
	size_t HeaderSize;

	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!Record)
		return KCF_ERROR_INVALID_PARAMETER;

	trace_kcf_msg("ReadRecord begin");
	trace_kcf_state(kcf);

	if (kcf->IsWriting || kcf->ReaderState != KCF_RDSTATE_RECORD_HEADER)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	kcf->AddedDataAlreadyRead = 0;
	kcf->AvailableAddedData   = 0;
	kcf->AddedDataCRC32       = 0;
	kcf->ActualAddedDataCRC32 = 0;
	Error = read_record_header(kcf, Record, &HeaderSize);
	if (Error)
		return Error;
	Record->DataSize = Record->HeadSize - HeaderSize;

	Record->Data = malloc(Record->DataSize);
	if (!Record->Data)
		return trace_kcf_error(KCF_ERROR_OUT_OF_MEMORY);

	if (!BIO_read_ex(kcf->Stream, Record->Data, Record->DataSize, NULL))
		return trace_kcf_error(KCF_ERROR_READ);

	trace_kcf_dump_buffer(Record->Data, Record->DataSize);

	if (rec_has_added_size(Record))
		kcf->ReaderState = KCF_RDSTATE_ADDED_DATA;
	else
		kcf->ReaderState = KCF_RDSTATE_RECORD_HEADER;

	kcf->AvailableAddedData = Record->AddedSize;

	trace_kcf_state(kcf);
	trace_kcf_msg("ReadRecord end");

	return KCF_ERROR_OK;
}

static 
int
BIO_skip(BIO *x, size_t size)
{
	int ret = 0;
	size_t n_read, to_read, remaining;
	uint8_t buffer[4096];

	n_read = 0;
	remaining = size;
	do
	{
		to_read = 4096;
		if (to_read > remaining)
			to_read = remaining;

		ret = BIO_read_ex(x, buffer, to_read, &n_read);
		remaining -= n_read;
		if (!ret)
			break;
		
	}
	while (remaining > 0);

	return ret;
}

KCFERROR KCF_skip_record(KCF *kcf)
{
	struct KcfRecord Header;
	size_t HeaderSize;
	uint64_t DataSize;
	KCFERROR Error;

	trace_kcf_msg("SkipRecord begin");
	trace_kcf_state(kcf);

	if (kcf->IsWriting)
		return KCF_ERROR_INVALID_STATE;
	if (kcf->ReaderState == KCF_RDSTATE_RECORD_HEADER) {
		Error = read_record_header(kcf, &Header, &HeaderSize);
		if (Error)
			return Error;

		DataSize = Header.HeadSize - HeaderSize + Header.AddedSize;
	} else if (kcf->ReaderState == KCF_RDSTATE_ADDED_DATA) {
		DataSize = kcf->AvailableAddedData;
	} else {
		return KCF_ERROR_INVALID_STATE;
	}

	if (!BIO_skip(kcf->Stream, DataSize))
		return KCF_ERROR_READ;

	kcf->ReaderState = KCF_RDSTATE_RECORD_HEADER;

	trace_kcf_state(kcf);
	trace_kcf_msg("SkipRecord end");

	return KCF_ERROR_OK;
}

bool KCF_is_added_data_available(KCF *kcf)
{
	if (!kcf)
		return false;

	return kcf->AvailableAddedData > 0;
}

KCFERROR KCF_read_added_data(KCF *kcf, void *Destination, size_t BufferSize,
                             size_t *BytesRead)
{
	size_t n_read;

	if (!kcf)
		return KCF_ERROR_INVALID_PARAMETER;

	if (!Destination)
		return KCF_ERROR_INVALID_PARAMETER;

	trace_kcf_msg("ReadAddedData begin");
	trace_kcf_state(kcf);

	if (kcf->IsWriting || kcf->ReaderState != KCF_RDSTATE_ADDED_DATA)
		return trace_kcf_error(KCF_ERROR_INVALID_STATE);

	if (BytesRead)
		*BytesRead = 0;

	if (kcf->AvailableAddedData == 0) {
		kcf->ReaderState = KCF_RDSTATE_RECORD_HEADER;
		trace_kcf_state(kcf);
		trace_kcf_msg("ReadAddedData end");
		return KCF_ERROR_OK;
	}

	if (kcf->AvailableAddedData < BufferSize)
		BufferSize = kcf->AvailableAddedData;

	if (!BIO_read_ex(kcf->Stream, Destination, BufferSize, &n_read) 
	    || n_read < BufferSize)
		return KCF_ERROR_READ;

	kcf->AvailableAddedData -= BufferSize;
	if (BytesRead)
		*BytesRead = n_read;
	kcf->ActualAddedDataCRC32 =
	    crc32c(kcf->ActualAddedDataCRC32, Destination, n_read);
	kcf->AddedDataAlreadyRead += n_read;

	if (kcf->AvailableAddedData == 0)
		kcf->ReaderState = KCF_RDSTATE_RECORD_HEADER;
	trace_kcf_state(kcf);
	trace_kcf_msg("ReadAddedData end");

	return KCF_ERROR_OK;
}
