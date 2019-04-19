#!/bin/bash

folder=$HOME

if [ ! -z $1 ]
then
    echo "Changing defaul folder to $1"
    folder=$1
    
fi

if [ ! -z $2 ] && [ -f $2 ]
then
    echo "Deleting previos $2"
    rm $2
fi

if [ ! -f "./pgrep" ]
then
    make
fi

echo "make has finished"

cores=(3 4 6 10 18 66 130)
for i in ${cores[*]}
do
    sample_size=0
    while [ $sample_size -lt 60 ]
    do
        exec 3>&1 4>&2
        tempo=$( { /usr/bin/time -f "%e" ./pgrep $i i $folder 1>/dev/null; } 2>&1 ) # change some_command
        exec 3>&- 4>&-
        sample_size=$((sample_size+1))
        
        if [ -z $2 ]
        then
            echo Using $i cores $tempo
            sample_size=60
        else
            echo $i $tempo >> $2
        fi

    done
done

make clean

