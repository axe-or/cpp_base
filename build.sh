set -eu

cc=clang++
cflags='-std=c++17 -O1 -fPIC -Wall -Wextra -fuse-ld=mold'

$cc $cflags demo.cpp base.cpp -o demo.bin
./demo.bin
