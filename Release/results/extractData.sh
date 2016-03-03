#!/bin/bash

file_in=$1
start_val=$2
number_of_vals=$3
step=$4
k=$start_val
for i in `seq $start_val $number_of_vals`
do
    echo "i: $i"
    echo "k: $k"
    fileName="result${k}.csv"
    sed -n "/k=$k)/,/Building/p" $1 | grep "Query avg. runtime" | cut -d' ' -f6 > $fileName
    sed -i -e "s/^/$k,/" $fileName
#    sed -i "1s/^/k,time\n/" $fileName
    k=`expr $k + $step`    
done
