tests: tests.c tap.c
	$(CC) $(CFLAGS) -o tests tests.c tap.c \
		../read.c ../record.c ../readbin.c ../marker.c \
		../kcf.c ../errors.c ../skip.c \
		../crc32c.c \
		asprintf.c \
		tests_marker.c \
		tests_read.c \
		tests_record.c \
		tests_validate.c

