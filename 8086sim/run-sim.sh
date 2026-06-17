#!/bin/bash

FILEPATH=$1 
RUN_CLEANUP_OPTION=$2 

BASE_FILENAME="${FILEPATH##*/}"
BASE_FILENAME="${BASE_FILENAME%.*}"
DIR="${FILEPATH%/*}"

echo $BASE_FILENAME 
echo $DIR 

gcc -g 8086sim.c -lm && \
./a.out "$DIR/$BASE_FILENAME.asm" > "temp" && \
echo "Comparison" && \
diff "temp" "./snapshots/$BASE_FILENAME" && \
echo "Done"

# Do cleanup in any case
echo "Cleanup" 
if [ "$RUN_CLEANUP_OPTION" = "-c" ]; then  
    rm "temp" "$BASE_FILENAME.out" "$BASE_FILENAME" "a.out" 2> /dev/null && \
    echo "Done"
fi 
