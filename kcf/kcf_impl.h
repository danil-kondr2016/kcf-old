#pragma once
#ifndef _KCF_IMPL_H_
#define _KCF_IMPL_H_

#include <kcf/archive.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>

#include "read.h"
#include "record.h"
#include "write.h"

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

enum KcfParserState
{
	KCF_PSTATE_NEUTRAL,

	KCF_PSTATE_READING = 0x100,
	KCF_PSTATE_READ_MARKER,
	KCF_PSTATE_READ_RECORD_HEADER,
	KCF_PSTATE_READ_RECORD_DATA,
	KCF_PSTATE_READ_ADDED_DATA,

	KCF_PSTATE_WRITING = 0x200,
	KCF_PSTATE_WRITE_MARKER,
	KCF_PSTATE_WRITE_RECORD,
	KCF_PSTATE_WRITE_ADDED_DATA,

	KCF_PSTATE_HIGH_MASK = 0xF00,
}; 

#define KCF_PSTATE_IS_WRITING(x) ( ((x)&KCF_PSTATE_HIGH_MASK) == KCF_PSTATE_WRITING )
#define KCF_PSTATE_IS_READING(x) ( ((x)&KCF_PSTATE_HIGH_MASK) == KCF_PSTATE_READING )

struct kcf_st { 
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

	IO *Stream;
	struct KcfRecord LastRecord;

	bool HasAddedDataCRC32 : 1;
	bool HasAddedSize      : 1;
	bool IsWriting         : 1;
	bool IsWritable        : 1;
	bool IsUnpacking       : 1;

	int  ParserState;

	union {
		enum KcfPackerState PackerState;
		enum KcfUnpackerState UnpackerState;
	};

	struct KcfFileInfo CurrentFile;
};

#ifdef _KCF_TRACE
static inline void trace_kcf_state(KCF *kcf)
{
	fputs("##KCFDEBUG## ", stderr);
	fprintf(stderr,
	        "%p %" PRId64 " %" PRId64 " %08" PRIx32 " %08" PRIx32
	        " %" PRId64 " %" PRId64 " %c%c %04X",
	        kcf, kcf->AvailableAddedData, kcf->AddedDataAlreadyRead,
	        kcf->AddedDataCRC32, kcf->ActualAddedDataCRC32,
	        kcf->RecordOffset, kcf->RecordEndOffset,
	        kcf->HasAddedDataCRC32 ? 'C' : '-',
	        kcf->HasAddedSize ? 'A' : '-',
	        kcf->ParserState);
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
	trace_kcf_msg("REC %04X %02X %02X %5u %20llu %08X", Record->HeadCRC,
	              Record->HeadType, Record->HeadFlags, Record->HeadSize,
	              Record->AddedSize, Record->AddedDataCRC32);
	trace_kcf_dump_buffer(Record->Data, Record->DataSize);
}
#else
static inline void trace_kcf_state(KCF *kcf) {}

static inline void trace_kcf_msg(const char *format, ...) {}

static inline void trace_kcf_dump_buffer(uint8_t *buf, size_t size) {}

static inline KCFERROR trace_kcf_error(KCFERROR Error) { return Error; }

static inline void trace_kcf_record(struct KcfRecord *Record) {}
#endif

#endif
