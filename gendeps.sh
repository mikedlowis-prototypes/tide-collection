#!/bin/sh

for f in "$@"; do
    fname="${f##*/}"    # Strip path
    fname="${fname%.*}" # Strip extension
    rule="bin/$fname: $f"
    for h in $(sed -nE 's/#include <(.+)>/\1/p' "$f"); do
        [[ -f "inc/$h" ]] && rule="$rule inc/$h"
    done
    echo "$rule"
done
