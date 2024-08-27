#pragma once
#ifndef _ARCHIVE_H_
#define _ARCHIVE_H_

#include <stdbool.h>
#include <stdint.h>

#include <io/io.h>

#include "errors.h"

typedef struct kcf_st KCF;

#define KCF_MODE_READ   0x01
#define KCF_MODE_CREATE 0x02
#define KCF_MODE_MODIFY 0x03

KCFERROR KCF_create(IO *stream, KCF **pkcf);
void KCF_close(KCF *kcf);

/* Write/read mode functions */

/**
 * Switches to write mode. Won't work if file is not open for writing
 * or modifying.
 */
bool KCF_start_writing(KCF *kcf);

/**
 * Switches to read mode. If any added data have been written, it will
 * be finished before switching to read mode.
 */
bool KCF_start_reading(KCF *kcf);

/* File inserting API */

enum KcfFileType {
	KCF_FILE_REGULAR   = 'R',
	KCF_FILE_DIRECTORY = 'd',
};

struct KcfFileInfo {
	uint64_t TimeStamp;
	uint64_t UnpackedSize;
	char *FileName;
	uint32_t FileCRC32;
	uint32_t CompressionInfo;
	enum KcfFileType FileType;
	bool HasTimeStamp     : 1;
	bool HasFileCRC32     : 1;
	bool HasUnpackedSize  : 1;
	bool HasUnpackedSize8 : 1;
};

bool file_info_copy(struct KcfFileInfo *Dest, struct KcfFileInfo *Src);
void file_info_clear(struct KcfFileInfo *info);

KCFERROR KCF_get_current_file_info(KCF *kcf, struct KcfFileInfo *FileInfo);
KCFERROR KCF_skip_file(KCF *kcf);
KCFERROR KCF_extract(KCF *kcf, IO *Output);

KCFERROR KCF_begin_file(KCF *kcf, struct KcfFileInfo *FileInfo);
KCFERROR KCF_insert_file_data(KCF *kcf, IO *Input);
KCFERROR KCF_end_file(KCF *kcf);

#endif
