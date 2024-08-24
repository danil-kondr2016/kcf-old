#pragma once
#ifndef _FILES_H_
#define _FILES_H_

#include "archive.h"
#include "record.h"
#include <openssl/bio.h>

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

KCFERROR file_info_to_record(struct KcfFileInfo *Info,
                             struct KcfRecord *Record);
KCFERROR record_to_file_info(struct KcfRecord *Record,
                             struct KcfFileInfo *Info);
bool file_info_copy(struct KcfFileInfo *Dest, struct KcfFileInfo *Src);
void file_info_clear(struct KcfFileInfo *info);

KCFERROR KCF_get_current_file_info(KCF *kcf, struct KcfFileInfo *FileInfo);
KCFERROR KCF_skip_file(KCF *kcf);
KCFERROR KCF_extract(KCF *kcf, BIO *Output);

KCFERROR KCF_begin_file(KCF *kcf, struct KcfFileInfo *FileInfo);
KCFERROR KCF_insert_file_data(KCF *kcf, BIO *Input);
KCFERROR KCF_end_file(KCF *kcf);

#endif
