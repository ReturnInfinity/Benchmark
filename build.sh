#!/bin/bash

# Build all benchmarks
gcc -o l_bench l_bench.c -lrt
gcc -o l_ethernet_bench l_ethernet_bench.c -lrt
nasm b_bench.asm -o b_bench.app
nasm b_ethernet_bench.asm -o b_ethernet_bench.app
