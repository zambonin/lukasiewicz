#!/bin/bash

if [[ -z "$1" ]] ; then
    echo "Usage: ./batch_test.sh (0?[1-9])|10"
    exit
fi

if [[ ! "$1" =~ ^0?[1-9]$|^10$ ]] ; then
    echo "Invalid argument."
    exit
fi

make
for i in $(seq -w 01 "$1") ; do
    diff <(./lukacompiler < "tests/$i.in") "tests/$i.out"
done
make clean
