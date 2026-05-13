#!/bin/bash

FILEPATH=$1 

BASE_FILENAME="${FILEPATH##*/}"
BASE_FILENAME="${BASE_FILENAME%.*}"
DIR="${FILEPATH%/*}"

nasm $FILEPATH && \
gcc -g 8086sim.c && \
./a.out "$DIR/$BASE_FILENAME" && \
cat "$DIR/$BASE_FILENAME.out" > temp && \
echo "; ========================================================================
; LISTING $BASE_FILENAME
; ========================================================================

bits 16
" > "$DIR/$BASE_FILENAME.out" && \
cat temp >> "$DIR/$BASE_FILENAME.out" && \
rm temp && \
echo "Comparison" && \
diff $FILEPATH "$DIR/$BASE_FILENAME.out" && \
echo "Done" && \
echo "Cleanup" && \
rm "$DIR/$BASE_FILENAME.out" "$DIR/$BASE_FILENAME" && \
echo "Done" 
