#!/bin/bash

RC_LIB=./build/lib
RC_RTILB_PATH=./

bitcode=$1
pure_functions_config=$2
con_level=$3

opt -load $RC_LIB/libresult-checking.so $bitcode -insert-guards -sensitive-functions $pure_functions_config -connectivity-level $con_level -o protected.bc

llvm-link protected.bc $RC_RTILB_PATH/response_lib.bc -o protected.bc

