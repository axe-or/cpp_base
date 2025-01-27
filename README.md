# C++ Base

Base utilities to writing C++

Requires C++20 for proper source location support Can be compiled using `ninja`
for incremental compilation or or just by doing a unity build of `src/lib.cpp`

Incremental build
```
ninja -j8
```

Unity build:
```
clang -Os -std=c++20 -I. -fPIC -Wall -Wextra -Wno-unused-label -c src/lib.cpp -o src/lib.cpp.o
ar rcs base.a src/lib.cpp.o
```


