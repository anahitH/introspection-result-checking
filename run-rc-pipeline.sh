#!/bin/bash

RC_LIB=./build/lib
RC_RTILB_PATH=./

pure_functions_config=$1
con_level=$2

~/build/llvm/Release/bin/clang -c -emit-llvm -W -Wall -Werror -DVERSION=\"1.0.1\" -o snake_3.5.bc snake.c
clang-3.9 -c -emit-llvm -W -Wall -Werror -DVERSION=\"1.0.1\" -o snake.bc snake.c
clang-3.9 -c -emit-llvm response_lib.c

opt-3.9 -load $RC_LIB/libresult-checking.so snake.bc -insert-guards -sensitive-functions $pure_functions_config -connectivity-level $con_level -o protected.bc

llvm-link-3.9 protected.bc $RC_RTILB_PATH/response_lib.bc -o protected.bc

clang-3.9 protected.bc -o protected
