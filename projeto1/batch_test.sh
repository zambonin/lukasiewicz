#!/bin/bash

if [[ -z "$1" ]] ; then
    echo "Usage: ./batch_test.sh 0?[1-8]"
    exit
fi

if [[ ! "$1" =~ ^0?[1-8]$ ]] ; then
    echo "Invalid argument."
    exit
fi

make
for i in $(seq -w 01 "$1") ; do
    if [[ -s "tests/valid/$i.in" && "tests/valid/$i.out" ]] ; then
        diff <(./lukacompiler < "tests/valid/$i.in") "tests/valid/$i.out"
    fi
done
make clean
