#!/bin/bash

source_name=$1
filename=$2
con_level=$3

clang $source_name.c -c -emit-llvm
clang ../response_lib.c -c -emit-llvm

opt -load ../build/lib/libresult-checking.so $source_name.bc -insert-guards -sensitive-functions $filename -connectivity-level $con_level -o protected.bc

llvm-link protected.bc response_lib.bc -o protected.bc

lli protected.bc

