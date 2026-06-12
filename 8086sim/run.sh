#!/bin/bash

FILEPATH=$1 
RUN_CLEANUP_OPTION=$2 

BASE_FILENAME="${FILEPATH##*/}"
BASE_FILENAME="${BASE_FILENAME%.*}"
DIR="${FILEPATH%/*}"

nasm $FILEPATH && \
gcc -g 8086decoder.c && \
./a.out "$DIR/$BASE_FILENAME" && \
# cleanup comments 
cat $FILEPATH > "$DIR/temp" && \
sed -i -E 's/;.*//; /^[[:space:]]*$/d' "$DIR/temp"  && \
echo "Comparison" && \
diff "$DIR/temp" "$DIR/$BASE_FILENAME.out" && \
echo "Done"

# Do cleanup in any case
echo "Cleanup" 
if [ "$RUN_CLEANUP_OPTION" = "-c" ]; then  
    rm "$DIR/temp" "$DIR/$BASE_FILENAME.out" "$DIR/$BASE_FILENAME" && \
    echo "Done"
fi 
