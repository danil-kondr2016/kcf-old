#pragma once
#ifndef _KCF_IMPL_H_
#define _KCF_IMPL_H_

#include "archive.h"
#include "files.h"
#include "kcferr.h"
#include "record.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

enum KcfWriterState {
	KCF_WRSTATE_IDLE,
	KCF_WRSTATE_MARKER,
	KCF_WRSTATE_RECORD,
	KCF_WRSTATE_ADDED_DATA,
};

enum KcfReaderState {
	KCF_RDSTATE_IDLE,
	KCF_RDSTATE_MARKER,
	KCF_RDSTATE_RECORD_HEADER,
	KCF_RDSTATE_RECORD_DATA,
	KCF_RDSTATE_ADDED_DATA,
};

enum KcfUnpackerState {
	KCF_UPSTATE_VALIDATING_FORMAT,
	KCF_UPSTATE_FILE_HEADER,
	KCF_UPSTATE_FILE_DATA,
	KCF_UPSTATE_AFTER_FILE_DATA,
};

enum KcfPackerState {
	KCF_PKSTATE_IDLE,
	KCF_PKSTATE_FILE_HEADER,
	KCF_PKSTATE_FILE_DATA,
	KCF_PKSTATE_AFTER_FILE_DATA,
};

struct Kcf {
	union {
		uint64_t AvailableAddedData;
		uint64_t AddedDataToBeWritten;
	};
	union {
		uint64_t AddedDataAlreadyRead;
		uint64_t WrittenAddedData;
	};
	uint32_t AddedDataCRC32;
	uint32_t ActualAddedDataCRC32;

	uint64_t RecordOffset;
	uint64_t RecordEndOffset;

	FILE *File;
	struct KcfRecord LastRecord;
	struct {
		bool HasAddedDataCRC32 : 1;
		bool HasAddedSize      : 1;
		bool IsWriting	       : 1;
		bool IsWritable	       : 1;
		bool IsUnpacking       : 1;
	};
	union {
		enum KcfWriterState WriterState;
		enum KcfReaderState ReaderState;
	};

	union {
		enum KcfPackerState PackerState;
		enum KcfUnpackerState UnpackerState;
	};

	struct KcfFileInfo CurrentFile;
};

#ifdef _KCF_TRACE
static inline void trace_kcf_state(HKCF hKCF)
{
	fputs("##KCFDEBUG## ", stderr);
	fprintf(stderr,
		"%p %" PRId64 " %" PRId64 " %08" PRIx32 " %08" PRIx32
		" %" PRId64 " %" PRId64 " %c%c%c %d",
		hKCF, hKCF->AvailableAddedData, hKCF->AddedDataAlreadyRead,
		hKCF->AddedDataCRC32, hKCF->ActualAddedDataCRC32,
		hKCF->RecordOffset, hKCF->RecordEndOffset,
		hKCF->HasAddedDataCRC32 ? 'C' : '-',
		hKCF->HasAddedSize ? 'A' : '-', hKCF->IsWriting ? 'W' : 'R',
		hKCF->ReaderState);
	fputc('\n', stderr);
}

static inline void trace_kcf_msg(const char *format, ...)
{
	va_list vl;

	fputs("##KCFDEBUG## ", stderr);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fputc('\n', stderr);
}

static inline void trace_kcf_dump_buffer(uint8_t *buf, size_t size)
{
	size_t i;

	fputs("##KCFDEBUG## Buffer dump ", stderr);
	fprintf(stderr, "at %p", buf);

	for (i = 0; i < size; i++) {
		if (i % 16 == 0) {
			fputs("\n##KCFDEBUG##", stderr);
		}
		fprintf(stderr, " %02X", buf[i]);
	}
	fputs("\n", stderr);
}

static inline KCFERROR trace_kcf_error(KCFERROR Error)
{
	fprintf(stderr, "##KCFDEBUG## Error #%d (%s)\n", Error,
		GetKcfErrorString(Error));
	return Error;
}

static inline void trace_kcf_record(struct KcfRecord *Record)
{
	trace_kcf_msg("REC %04X %02X %02X %5u %20llu %08X",
		      Record->Header.HeadCRC, Record->Header.HeadType,
		      Record->Header.HeadFlags, Record->Header.HeadSize,
		      Record->Header.AddedSize, Record->Header.AddedDataCRC32);
	trace_kcf_dump_buffer(Record->Data, Record->DataSize);
}
#else
static inline void trace_kcf_state(HKCF hKCF) {}

static inline void trace_kcf_msg(const char *format, ...) {}

static inline void trace_kcf_dump_buffer(uint8_t *buf, size_t size) {}

static inline KCFERROR trace_kcf_error(KCFERROR Error) { return Error; }

static inline void trace_kcf_record(struct KcfRecord *Record) {}
#endif

#endif
