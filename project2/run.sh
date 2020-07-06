#!/bin/bash
chmod 777 ./montecarlo/circle
chmod 777 ./quicksort/quicksortomp
chmod 777 ./pagerank/pagerankomp

if test $1 = "mc"; then
    ./montecarlo/circle $2
elif test $1 = "qs"; then
    ./quicksort/quicksortomp $2
elif test $1 = "pr"; then
    ./pagerank/pagerankomp $2 $3
fi