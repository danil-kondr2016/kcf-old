#include <kcf/archive.h>
#include <kcf/errors.h>

#include "kcf_impl.h"

#define MARKER_1 'K'
#define MARKER_2 'C'
#define MARKER_3 '!'
#define MARKER_4 0x1A
#define MARKER_5 0x06
#define MARKER_6 0x00

KCFERROR KCF_find_marker(KCF *kcf)
{
	uint8_t buf[6] = {0};
	int ret;

	if (kcf->IsWriting || kcf->ReaderState != KCF_RDSTATE_MARKER)
		return KCF_ERROR_INVALID_STATE;

	do {
		if (buf[0] == MARKER_1 && buf[1] == MARKER_2 &&
		    buf[2] == MARKER_3 && buf[3] == MARKER_4 &&
		    buf[4] == MARKER_5 && buf[5] == MARKER_6)
			goto ok;

		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = buf[3];
		buf[3] = buf[4];
		buf[4] = buf[5];

		ret = IO_read(kcf->Stream, &buf[5], 1);
	} while (ret > 0);

	if (ret == -1)
		return KCF_ERROR_READ;

	return KCF_ERROR_INVALID_FORMAT;
ok:
	kcf->ReaderState = KCF_RDSTATE_RECORD_HEADER;
	return KCF_ERROR_OK;
}

KCFERROR KCF_write_marker(KCF *kcf)
{
	uint8_t marker[6];
	
	if (!kcf->IsWriting || kcf->WriterState != KCF_WRSTATE_MARKER)
		return KCF_ERROR_INVALID_STATE;

	marker[0] = MARKER_1;
	marker[1] = MARKER_2;
	marker[2] = MARKER_3;
	marker[3] = MARKER_4;
	marker[4] = MARKER_5;
	marker[5] = MARKER_6;

	if (IO_write(kcf->Stream, marker, 6) < 0)
		return KCF_ERROR_WRITE;

	kcf->WriterState = KCF_WRSTATE_IDLE;
	return KCF_ERROR_OK;
}
