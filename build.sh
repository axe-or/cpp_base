set -eu

cc=clang++
cflags='-std=c++20 -O0 -fPIC -Wall -Wextra -fuse-ld=mold'
ignoreflags='-Wno-unused-label'
ldflags='-L.'

cflags="$cflags $ignoreflags"

time $cc $cflags demo.cpp base.cpp -o demo.bin $ldflags
./demo.bin

