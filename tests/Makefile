FLAG_KCF_TRACE_0 := 
FLAG_KCF_TRACE_1 := -D_KCF_TRACE
FLAG_KCF_TRACE = $(FLAG_KCF_TRACE_$(KCF_TRACE))

tests: tests.c tap.c
	$(CC) $(CFLAGS) $(FLAG_KCF_TRACE) -o tests tests.c tap.c \
		../read.c ../record.c ../bytepack.c ../marker.c \
		../filehdr.c ../archhdr.c ../write.c \
		../kcf.c ../errors.c ../skip.c \
		../crc32c.c \
		asprintf.c \
		tests_marker.c \
		tests_read.c \
		tests_record.c \
		tests_validate.c

puthello: puthello.c
	$(CC) $(CFLAGS) $(FLAG_KCF_TRACE) -o puthello puthello.c \
		../read.c ../record.c ../bytepack.c ../marker.c \
		../filehdr.c ../archhdr.c ../write.c \
		../kcf.c ../errors.c ../skip.c \
		../crc32c.c 
