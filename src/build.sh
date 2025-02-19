#!/usr/bin/env sh

cc=clang++
cflags='-std=c++17 -O1 -pipe -fPIC -Wall -Wextra -fsanitize=address'

set -eu

Run(){ echo "$@" > /dev/stderr ; $@; }

Run $cc $cflags main.cpp base.cpp -o demo.elf

./demo.elf

