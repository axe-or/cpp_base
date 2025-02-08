#!/usr/bin/env sh

cc=clang++
cflags='-std=c++20 -O0 -g -pipe -fPIC -Wall -Wextra'

set -eu

Run(){ echo "$@"; $@; }

Run $cc $cflags main.cpp base.cpp -o demo.elf

./demo.elf

