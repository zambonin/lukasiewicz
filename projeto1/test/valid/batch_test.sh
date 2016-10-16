#!/bin/bash

if [[ -z "$1" ]] ; then
    echo "Usage: ./batch_test.sh /path/to/lukacompiler"
    exit
fi

bold=$(tput bold)
normal=$(tput sgr0)

for f in */*.in ; do
    diff -B <("$1" < "$f") "${f/in/ex}"
    (("$?" > 0)) && echo -e "${bold}$f${normal} is not correct\n"
done
