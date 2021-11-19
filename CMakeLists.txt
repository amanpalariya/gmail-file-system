cmake_minimum_required(VERSION 3.8)

project(Assignment5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("/usr/include/fuse3")
link_libraries(-lfuse3 -lpthread -lcurl)

add_executable(main.out main.c fs.c fs.h logger.c logger.h configure.c configure.h)
add_executable(test.out tests/tester.c fs.c fs.h logger.c logger.h configure.c configure.h)
