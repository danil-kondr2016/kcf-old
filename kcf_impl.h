#pragma once
#include "kcf.h"
#include "errors.h"
#include "record.h"
#ifndef _KCF_IMPL_H_
#define _KCF_IMPL_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

enum KcfWriterState
{
	KCF_STATE_WRITING_IDLE,
	KCF_STATE_WRITING_MARKER,
	KCF_STATE_WRITING_MAIN_RECORD,
	KCF_STATE_WRITING_ADDED_DATA,
};

enum KcfReaderState
{
	KCF_STATE_READING_IDLE,
	KCF_STATE_SEEKING_MARKER,
	KCF_STATE_AT_THE_BEGINNING_OF_RECORD,
	KCF_STATE_AT_MAIN_FIELD_OF_RECORD,
	KCF_STATE_AT_ADDED_FIELD_OF_RECORD,
};

struct Kcf
{
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
		bool HasAddedSize : 1;
		bool IsWriting : 1;
		bool IsWritable : 1;
	};
	union {
		enum KcfWriterState WriterState;
		enum KcfReaderState ReaderState;
	};
};

#ifdef _KCF_TRACE
static inline
void trace_kcf_state(HKCF hKCF)
{
	fputs("##KCFDEBUG## ", stderr);
	fprintf(stderr, "%p %" PRId64 " %" PRId64 " %08" PRIx32 " %08" PRIx32 " %" PRId64 " %" PRId64 " %c%c%c %d", 
		hKCF, hKCF->AvailableAddedData, hKCF->AddedDataAlreadyRead, hKCF->AddedDataCRC32,
		hKCF->ActualAddedDataCRC32, hKCF->RecordOffset, hKCF->RecordEndOffset,
		hKCF->HasAddedDataCRC32 ? 'C' : '-',
		hKCF->HasAddedSize      ? 'A' : '-',
		hKCF->IsWriting         ? 'W' : 'R',
		hKCF->ReaderState
		);
	fputc('\n', stderr);
}

static inline
void trace_kcf_msg(const char *format, ...)
{
	va_list vl;

	fputs("##KCFDEBUG## ", stderr);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fputc('\n', stderr);
}

static inline
void trace_kcf_dump_buffer(uint8_t *buf, size_t size)
{
	size_t i;

	fputs("##KCFDEBUG## Buffer dump ", stderr);
	fprintf(stderr, "at %p", buf);

	for (i = 0; i < size; i++) {
		if (i % 16 == 0) {
			fputs("\n##KCFDEBUG##", stderr);
		}
		fprintf(stderr, " %02X",  buf[i]);
	}
	fputs("\n", stderr);
}

static inline
KCFERROR trace_kcf_error(KCFERROR Error)
{
	fprintf(stderr, "##KCFDEBUG## Error #%d (%s)\n", Error, GetKcfErrorString(Error));
	return Error;
}
#else
#define trace_kcf_state(x)
#define trace_kcf_msg(...)
#define trace_kcf_dump_buffer(a, b)
#define trace_kcf_error(e) (e)
#endif

#endif
