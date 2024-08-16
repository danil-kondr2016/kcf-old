#include "errors.h"
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

	if (hKCF->IsWriting || hKCF->ReaderState != KCF_STATE_SEEKING_MARKER)
		return KCF_ERROR_INVALID_STATE;

	do {
		if (buf[0] == MARKER_1
			&& buf[1] == MARKER_2
			&& buf[2] == MARKER_3
			&& buf[3] == MARKER_4
			&& buf[4] == MARKER_5
			&& buf[5] == MARKER_6)
			goto ok;

		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = buf[3];
		buf[3] = buf[4];
		buf[4] = buf[5];

		n_read = fread(&buf[5], 1, 1, hKCF->File);
	}
	while (n_read);

	if (ferror(hKCF->File))
		return KCF_ERROR_READ;

	return KCF_ERROR_INVALID_FORMAT;
ok:
	hKCF->ReaderState = KCF_STATE_AT_THE_BEGINNING_OF_RECORD;
	return KCF_ERROR_OK;
}

KCFERROR WriteArchiveMarker(HKCF hKCF)
{
	if (!hKCF->IsWriting || hKCF->WriterState != KCF_STATE_WRITING_MARKER)
		return KCF_ERROR_INVALID_STATE;

	if (fputc(MARKER_1, hKCF->File) == EOF) return KCF_ERROR_WRITE;
	if (fputc(MARKER_2, hKCF->File) == EOF) return KCF_ERROR_WRITE;
	if (fputc(MARKER_3, hKCF->File) == EOF) return KCF_ERROR_WRITE;
	if (fputc(MARKER_4, hKCF->File) == EOF) return KCF_ERROR_WRITE;
	if (fputc(MARKER_5, hKCF->File) == EOF) return KCF_ERROR_WRITE;
	if (fputc(MARKER_6, hKCF->File) == EOF) return KCF_ERROR_WRITE;

	hKCF->WriterState = KCF_STATE_WRITING_IDLE;
	return KCF_ERROR_OK;
}
