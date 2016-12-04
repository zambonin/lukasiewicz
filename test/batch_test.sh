#!/bin/bash

if [[ -z "$1" ]] ; then
    echo "Usage: ./batch_test.sh /path/to/lukacompiler"
    exit
fi

bold=$(tput bold)
normal=$(tput sgr0)

for f in valid/*/*.in ; do
    cmp -s <("$1" < "$f") "${f/.in/.out}"
    (("$?" > 0)) && echo -e "${bold}$f${normal} is not correct"
    CNT="$(valgrind "$1" < "$f" 2>&1 | grep -E "(exit|SUMMARY): 0" | wc -l)"
    (( "$CNT" < 2 )) && echo -e "${bold}$f${normal} is leaking memory"
done

for f in invalid/*/*.in ; do
    "$1" < "$f" 1>/dev/null 2> "${f/.in/.out}"
done

for f in valid/*/*.in ; do
    PYFILE="${f/.in/.py}"
    "$1" -p < "$f" > "$PYFILE"
    sed -i 's/src/..\/..\/..\/src/' "$PYFILE"
    cd "$(dirname "$PYFILE")"
    python "$(basename "$PYFILE")"
    cd - >/dev/null
done
