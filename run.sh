#! /bin/bash

cd build

clang -c --target=spirv64-unknown-unknown -cl-std=CL3.0 -cl-fast-relaxed-math -O3 -emit-llvm \
    ../src/kernels/grayscale.cl -o grayscale.bc
llvm-spirv --spirv-max-version=1.0 grayscale.bc -o ../src/kernels/spirv/grayscale.cl.spv

clang -O3 -flto -lpng -lOpenCL -Wall ../src/main.c -o main

cp -r ../src/kernels kernels

./main