#include "errors.h"

#include <errno.h>

KCFERROR ErrnoToKcf()
{
	int Error = errno;

	switch (Error) {
	case ENOENT:
		return KCF_ERROR_FILE_NOT_FOUND;
	case EACCES:
		return KCF_ERROR_ACCESS_DENIED;
	case EINVAL:
		return KCF_ERROR_INVALID_PARAMETER;
	case ENOMEM:
		return KCF_ERROR_OUT_OF_MEMORY;
	default:
		return KCF_ERROR_UNKNOWN;
	}
}

KCFERROR FileErrorToKcf(FILE *File, enum KcfFileSituation Situation)
{
	if (!File)
		return KCF_ERROR_OK;

	switch (Situation) {
	case KCF_SITUATION_WRITING:
		if (ferror(File))
			return KCF_ERROR_WRITE;
		break;
	case KCF_SITUATION_READING_IN_BEGINNING:
		if (ferror(File))
			return KCF_ERROR_READ;
		else if (feof(File))
			return KCF_ERROR_EOF;
		break;
	case KCF_SITUATION_READING_IN_MIDDLE:
		if (ferror(File))
			return KCF_ERROR_READ;
		else if (feof(File))
			return KCF_ERROR_PREMATURE_EOF;
		break;	
	}

	return KCF_ERROR_OK;
}

