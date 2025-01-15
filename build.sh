set -eu

cc=clang++
cflags='-std=c++17 -O1 -fPIC -Wall -Wextra -fuse-ld=mold'
ignoreflags='-Wno-unused-label'
ldflags='-L.'

cflags="$cflags $ignoreflags"

$cc $cflags demo.cpp base.cpp -o demo.bin $ldflags
./demo.bin

