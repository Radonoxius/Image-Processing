#! /bin/bash

cd build

clang -O3 -flto -lpng -lOpenCL -Wall ../src/main.c -o main

./main