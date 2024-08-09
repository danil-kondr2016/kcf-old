#include "utils.h"

int kcf_fskip(FILE *File, size_t SizeToSkip)
{
	int Seekable = 1;

	if (fseek(File, 0L, SEEK_CUR) == -1)
		Seekable = 0;

	if (Seekable) {
		if (fseeko(File, SizeToSkip, SEEK_CUR) == -1)
			return -1;
	}
	else {
		char buf[1024];
		size_t size = SizeToSkip;

		while (DataSize >= sizeof(buf)) {
			size -= fread(buf, 1, sizeof(buf), File);	
			if (ferror(File))
				return -1;
		}
		while (size > 0) {
			size -= fread(buf, 1, to_read, File);
			if (ferror(File))
				return -1;
		}
	}

	return 0;
}
