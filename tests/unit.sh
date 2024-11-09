#!/bin/bash

RED="\e[31m"
GREEN="\e[32m"
WHITE='\e[37m'
BOLD="\e[1m"
RESET="\e[0m"

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
  echo -e "Running ${BOLD}$test${RESET}"

  $test
  if [ $? -ne 0 ]; then
    echo -e "${BOLD}[ ${RED}FAIL${WHITE} ]${RESET} $test"
    ((failed++))
  else
    echo -e "${BOLD}[ ${GREEN}PASS${WHITE} ]${RESET} $test"
  fi

  ((total++))
done

printf "%i/%i tests passed\n" $(expr $total - $failed) $total
if [[ $failed != 0 ]]; then
  exit 1
fi
