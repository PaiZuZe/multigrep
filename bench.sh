#!/bin/bash

folder=$HOME

if [ ! -z $1 ]
then
    echo "Changing defaul folder to $1"
    folder=$1
    
fi

if [ ! -f "./pgrep" ]
then
    make
fi

cores=(1 2 4 8 16 64 128)
for i in ${cores[*]}
do
    exec 3>&1 4>&2
    tempo=$( { /usr/bin/time -f "%e" ./pgrep $i i $folder >> /dev/null; } 2>&1 ) # change some_command
    exec 3>&- 4>&-
    echo "Using $i cores $tempo"
done

make clean

