#pragma once
#ifndef _STDIO64_H_
#define _STDIO64_H_

#define kcf_fclose fclose
#define kcf_fread  fread
#define kcf_fwrite fwrite
#define kcf_feof   feof
#define kcf_ferror ferror

#if defined(KCF_WINDOWS) || defined(_WIN32) || defined(_WIN64)

#include "msapi_utf8.h"
#include <stdio.h>

#define kcf_fopen fopenU
#define kcf_fseek _fseeki64
#define kcf_ftell _ftelli64

#elif defined(KCF_UNIX)

#define _FILE_OFFSET_BITS 64
#include <stdio.h>

#define kcf_fopen fopen
#define kcf_fseek fseeko64
#define kcf_ftell ftello64

#else
#error "Unrecognized platform"
#endif

#endif
