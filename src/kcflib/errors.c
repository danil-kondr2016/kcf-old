#include <kcf/errors.h>

#include <errno.h>

KCFERROR kcf_errno()
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

KCFERROR kcf_file_error(FILE *File, enum KcfFileSituation Situation)
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

static const char *_KcfErrorStrings[KCF_ERROR_MAX] = {
    [KCF_ERROR_OK]                = "Success",
    [KCF_ERROR_UNKNOWN]           = "Unknown error",
    [KCF_ERROR_NOT_IMPLEMENTED]   = "Function not implemented",
    [KCF_ERROR_INVALID_PARAMETER] = "Invalid parameter",
    [KCF_ERROR_INVALID_FORMAT]    = "Invalid archive format",
    [KCF_ERROR_INVALID_DATA]      = "Invalid data",
    [KCF_ERROR_INVALID_STATE]     = "Invalid state",
    [KCF_ERROR_FILE_NOT_FOUND]    = "File not found",
    [KCF_ERROR_ACCESS_DENIED]     = "Access denied",
    [KCF_ERROR_OUT_OF_MEMORY]     = "Out of memory",
    [KCF_ERROR_WRITE]             = "Write error",
    [KCF_ERROR_READ]              = "Read error",
    [KCF_ERROR_EOF]               = "End of file",
    [KCF_ERROR_PREMATURE_EOF]     = "Premature end of file",
};

const char *kcf_error_string(KCFERROR Error)
{
	if (Error < KCF_ERROR_OK || Error >= KCF_ERROR_MAX)
		return _KcfErrorStrings[KCF_ERROR_UNKNOWN];

	return _KcfErrorStrings[Error];
}
