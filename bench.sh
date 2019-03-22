#!/bin/bash

if [ ! -f "./pgrep" ]
then
    make
fi

cores=(1 2 4 8 16 64 128)
for i in ${cores[*]}
do
    exec 3>&1 4>&2
    tempo=$( { /usr/bin/time -f "%e" ./pgrep $i i ~/Documents/3ºano/ >> /dev/null; } 2>&1 ) # change some_command
    exec 3>&- 4>&-
    echo "Using $i cores $tempo"
done

make clean

