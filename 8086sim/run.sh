#!/bin/bash

FILEPATH=$1 

BASE_FILENAME="${FILEPATH##*/}"
BASE_FILENAME="${BASE_FILENAME%.*}"
DIR="${FILEPATH%/*}"

nasm $FILEPATH && \
gcc -g 8086sim.c && \
./a.out "$DIR/$BASE_FILENAME" && \
# cleanup comments 
cat $FILEPATH > "$DIR/temp" && \
sed -i -E 's/;.*//; /^[[:space:]]*$/d' "$DIR/temp"  && \
echo "Comparison" && \
diff "$DIR/temp" "$DIR/$BASE_FILENAME.out" && \
echo "Done"

# Do cleanup in any case
echo "Cleanup" && \
rm "$DIR/temp" "$DIR/$BASE_FILENAME.out" "$DIR/$BASE_FILENAME" && \
echo "Done" 
