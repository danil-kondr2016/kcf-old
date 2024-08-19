#include "tap.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../archive.h"
#include "../record.h"

extern HKCF hKCF;

bool test1(void);
bool test2(void);
bool test3(void);
bool test4(void);
bool test5(void);
bool test6(void);
bool test7(void);
bool test8(void);
bool test9(void);
bool test10(void);
bool test11(void);
bool test12(void);

int main(void)
{
	plan_tests(12);
	ok(test1(), "file with valid marker");
	ok(test2(), "file without valid marker");
	ok(test3(), "read archive header record");
	ok(test4(), "skip one record and read");
	ok(test5(), "read record with added size");
	ok(test6(), "read added data");
	ok(test7(), "read chunk after added data");
	ok(test8(), "record to archive header (valid)");
	ok(test9(), "record to archive header (invalid by type)");
	ok(test10(), "record to archive header (invalid by size)");
	ok(test11(), "validate CRC of archive header");
	ok(test12(), "record to file header (valid)");

	if (hKCF)
		CloseArchive(hKCF);

	return exit_status();
}
