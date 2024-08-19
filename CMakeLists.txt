cmake_minimum_required(VERSION 3.20)
project(KCF VERSION 0.0.1 LANGUAGES C)
add_compile_definitions(_KCF_TRACE)
add_executable(kcf main.c archive.c archhdr.c bytepack.c crc32c.c errors.c filehdr.c marker.c pack.c read.c record.c skip.c unpack.c write.c)