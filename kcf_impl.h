#pragma once
#include "record.h"
#ifndef _KCF_IMPL_H_
#define _KCF_IMPL_H_

#include <stdio.h>
#include <stdint.h>

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
	};
	int WriterState;
};

enum KcfWriterState
{
	KCF_STATE_WRITING_MARKER,
	KCF_STATE_WRITING_MAIN_RECORD,
	KCF_STATE_WRITING_ADDED_DATA,
};


#endif
