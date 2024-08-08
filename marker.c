#include "record.h"
#include "kcf.h"

#include "kcf_impl.h"

#define MARKER_1 'K'
#define MARKER_2 'C'
#define MARKER_3 '!'
#define MARKER_4 0x1A
#define MARKER_5 0x06
#define MARKER_6 0x00

KCFERROR ScanArchiveForMarker(HKCF hKCF)
{
	uint8_t buf[6] = {0};
	size_t n_read = 0;

	do {
		if (buf[0] == MARKER_1
			&& buf[1] == MARKER_2
			&& buf[2] == MARKER_3
			&& buf[3] == MARKER_4
			&& buf[4] == MARKER_5
			&& buf[5] == MARKER_6)
			return KCF_ERROR_OK;

		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = buf[3];
		buf[3] = buf[4];
		buf[4] = buf[5];

		n_read = fread(&buf[5], 1, 1, hKCF->File);
	}
	while (n_read);

	return KCF_ERROR_INVALID_FORMAT;
}

#ifdef TEST_MARKER
int main(void)
{
	HKCF hKCF;
	KCFERROR Error;

	fputs("Marker,1,good,",stdout);

	Error = CreateArchive("tests/mk1g.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		fputs("not_found\n", stdout);
		fputs("Failed to conduct test #1\n", stderr);
		return 1;
	}

	Error = ScanArchiveForMarker(hKCF);
	if (Error == KCF_ERROR_OK) {
		fputs("pass\n", stdout);
	}
	else {
		fputs("fail\n", stdout);
	}

	CloseArchive(hKCF);

	fputs("Marker,2,bad,", stdout);
	Error = CreateArchive("tests/mk2b.kcf", KCF_MODE_READ, &hKCF);
	if (Error) {
		fputs("not_found\n", stdout);
		fputs("Failed to conduct test #2\n", stderr);
		return 1;
	}

	Error = ScanArchiveForMarker(hKCF);
	if (Error == KCF_ERROR_INVALID_FORMAT) {
		fputs("pass\n", stdout);
	}
	else {
		fputs("fail\n", stdout);
	}

	CloseArchive(hKCF);

	return 0;
}
#endif
