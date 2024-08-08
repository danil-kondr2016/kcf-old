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
