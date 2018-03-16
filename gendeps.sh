#!/bin/sh

for f in "$@"; do
    fname="${f##*/}"    # Strip path
    fname="${fname%.*}" # Strip extension
    rule="bin/$fname: $f"
    for h in $(grep '#include' "$f" | sed -E 's/#include <(.+)>/\1/'); do
        [[ -f "inc/$h" ]] && rule="$rule inc/$h"
    done
    echo "$rule"
done
