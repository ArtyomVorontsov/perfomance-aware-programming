#!/bin/sh

nasm 8086.asm && \
gcc -g 8086sim.c && \
./a.out
