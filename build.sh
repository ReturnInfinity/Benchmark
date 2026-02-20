#!/bin/bash

BMCFLAGS="-c -m64 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -falign-functions=16 -fomit-frame-pointer -mno-red-zone -fno-builtin -fno-stack-protector -ffunction-sections -fdata-sections"

# Build all benchmarks
if [ "$(uname)" != "Darwin" ]; then
	gcc -o l_bench l_bench.c -lrt
	gcc -o l_ethernet_bench l_ethernet_bench.c -lrt
	gcc -o l_raytrace l_raytrace.c
	gcc $BMCFLAGS -o crt0.o crt0.c
	gcc $BMCFLAGS -o b_bench.o b_bench.c
	gcc $BMCFLAGS -o b_ethernet_bench.o b_ethernet_bench.c
	gcc $BMCFLAGS -o b_raytrace.o b_raytrace.c
	gcc $BMCFLAGS -o libBareMetal.o libBareMetal.c
	ld -T c.ld -o b_bench.app crt0.o b_bench.o libBareMetal.o
	ld -T c.ld -o b_ethernet_bench.app crt0.o b_ethernet_bench.o libBareMetal.o
	ld -T c.ld -o b_raytrace.app crt0.o b_raytrace.o libBareMetal.o
else
	echo "Unable to build BareMetal C programs on macOS. Building remainder."
fi

cd assembly
nasm b_bench.asm -o b_bench.app
nasm b_ethernet_bench.asm -o b_ethernet_bench.app
cd ..
