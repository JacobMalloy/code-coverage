#!/bin/bash
mkdir -p build && cd build
cmake -DLLVM_DIR=$(llvm-config --cmakedir) -DCMAKE_BUILD_TYPE=Debug ..
make

