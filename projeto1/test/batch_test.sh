#!/bin/bash

if [[ -z "$1" ]] ; then
    echo "Usage: ./batch_test.sh /path/to/lukacompiler"
    exit
fi

bold=$(tput bold)
normal=$(tput sgr0)

for f in valid/*/*.in ; do
    cmp -s <("$1" < "$f") "${f/.in/.ex}"
    (("$?" > 0)) && echo -e "${bold}$f${normal} is not correct"
    CNT="$(valgrind "$1" < "$f" 2>&1 | grep -E "(exit|SUMMARY): 0" | wc -l)"
    (( "$CNT" < 2 )) && echo -e "${bold}$f${normal} is leaking memory"
done

for f in invalid/*/*.in ; do
    "$1" < "$f" 1>/dev/null 2> "${f/.in/.ex}"
done
