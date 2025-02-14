#!/usr/bin/env

set -xeu

clang -O3 -fPIC -Wall -Wextra src/static.c -c -o mimalloc.o -Iinclude
ar rcs mimalloc.a mimalloc.o

