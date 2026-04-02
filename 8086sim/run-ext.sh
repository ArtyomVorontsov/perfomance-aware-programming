#!/bin/sh

nasm 8086ext.asm && \
gcc -g 8086sim.c && \
./a.out && \
cat 8086ext.out > temp && \
echo "; ========================================================================
; LISTING 38
; ========================================================================

bits 16
" > 8086ext.out && \
cat temp >> 8086ext.out && \
rm temp && \
diff 8086ext.asm 8086ext.out && \
echo "End of comparison"
