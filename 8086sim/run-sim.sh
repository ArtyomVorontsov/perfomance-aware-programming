#!/bin/bash

LD_LIBRARY_PATH=./shared:$LD_LIBRARY_PATH
FILEPATH=$1 
RUN_CLEANUP_OPTION=$2 

BASE_FILENAME="${FILEPATH##*/}"
BASE_FILENAME="${BASE_FILENAME%.*}"
DIR="${FILEPATH%/*}"

echo $BASE_FILENAME 
echo $DIR 

g++ -g 8086sim.cpp -lm -L./shared -lsim86_lib
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
