#!/usr/bin/env sh

cc=clang++
cflags='-std=c++17 -O0 -g -pipe -fPIC -Wall -Wextra -DUSE_MIMALLOC -fsanitize=address'

set -eu

Run(){ echo "$@"; $@; }

MiMalloc="dep/mimalloc/mimalloc.o"
[ -f  "$MiMalloc" ] || {
	Run clang -std=c17 -O3 -fPIC -c dep/mimalloc/src/static.c -I dep/mimalloc/include -o "$MiMalloc"
	Run ar rcs mimalloc.a "$MiMalloc"
}

Run $cc $cflags main.cpp base.cpp -o demo.elf mimalloc.a

./demo.elf

