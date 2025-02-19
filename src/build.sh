#!/usr/bin/env sh

cc=clang++
cflags='-std=c++17 -Wall -Wextra'

buildMode="$1"

case "$buildMode" in
	'release') cflags="$cflags -DRELEASE_MODE -O3";;
	*)         cflags="$cflags -O0 -g" ;;
esac

set -eu

Run(){ echo "$@"; $@; }

Run $cc $cflags main.cpp base.cpp -o demo.exe

./demo.exe

