#pragma once
#ifndef _FILES_H_
#define _FILES_H_

#include "archive.h"
#include "record.h"
#include <openssl/bio.h>

enum KcfFileType
{
	KCF_FILE_REGULAR = 'R',
	KCF_FILE_DIRECTORY = 'd',
};

struct KcfFileInfo
{
	uint64_t TimeStamp;
	uint64_t UnpackedSize;
	char     *FileName;	
	uint32_t FileCRC32;
	uint32_t CompressionInfo;
	enum KcfFileType FileType;
	bool     HasTimeStamp     : 1;
	bool     HasFileCRC32     : 1;
	bool     HasUnpackedSize  : 1;
	bool     HasUnpackedSize8 : 1;
};

KCFERROR FileInfoToRecord(
	struct KcfFileInfo *Info, 
	struct KcfRecord *Record
);
KCFERROR RecordToFileInfo(
	struct KcfRecord *Record, 
	struct KcfFileInfo *Info
);

KCFERROR GetCurrentFileInfo(HKCF hKCF, struct KcfFileInfo *FileInfo);
KCFERROR SkipFile(HKCF hKCF);
KCFERROR ExtractFileData(HKCF hKCF, BIO *Output);

KCFERROR BeginFile(HKCF hKCF, struct KcfFileInfo *FileInfo);
KCFERROR InsertFileData(HKCF hKCF, BIO *Input);
KCFERROR EndFile(HKCF hKCF);

#endif
