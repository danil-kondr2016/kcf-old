#pragma once
#ifndef _KCF_IMPL_H_
#define _KCF_IMPL_H_

#include <stdio.h>
#include <stdint.h>

struct Kcf
{
	uint64_t AvailableAddedData;
	uint64_t AddedDataAlreadyRead;
	uint32_t AddedDataCRC32;
	uint32_t ActualAddedDataCRC32;

	FILE *File;
	
};

#endif
