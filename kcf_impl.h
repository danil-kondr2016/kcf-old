#pragma once
#ifndef _KCF_IMPL_H_
#define _KCF_IMPL_H_

#include <stdio.h>
#include <stdint.h>

struct Kcf
{
	FILE *File;
	uint64_t AvailableAddedData;
};

#endif
