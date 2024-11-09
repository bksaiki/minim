#!/bin/bash

RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

# determine physical directory of this script
src="${BASH_SOURCE[0]}"
while [ -L "$src" ]; do
  dir="$(cd -P "$(dirname "$src")" && pwd)"
  src="$(readlink "$src")"
  [[ $src != /* ]] && src="$dir/$src"
done
DIR="$(cd -P "$(dirname "$src")" && pwd)"

TOPDIR="$DIR/.."
TESTS="$@"


##  Unit tests


failed=0
total=0

echo "Running unit tests"

for file in $TESTS; do
  test="$(realpath $TOPDIR/$file)"
  echo "Running $test"

  $test
  if [ $? -ne 0 ]; then
    ((failed++))
  fi

  ((total++))
done

printf "%i/%i tests passed\n" $(expr $total - $failed) $total
if [[ $failed != 0 ]]; then
  exit 1
fi
