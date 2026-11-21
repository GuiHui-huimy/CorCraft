#!/usr/bin/env bash
# Runs every labs/<track>/<lab>/check.sh and prints one line per lab.
# Exit status is 0 only when all labs pass.

set -u

root="$(cd "$(dirname "$0")/.." && pwd)"
total=0
failed=0

for script in "$root"/labs/*/*/check.sh; do
  if [ ! -f "$script" ]; then
    continue
  fi
  total=$((total + 1))
  rel="${script#"$root"/}"
  if bash "$script" >/dev/null 2>&1; then
    printf 'PASS  %s\n' "$rel"
  else
    printf 'FAIL  %s\n' "$rel"
    failed=$((failed + 1))
  fi
done

printf '\n%d lab check(s), %d failed\n' "$total" "$failed"

if [ "$total" -eq 0 ]; then
  printf 'Nothing to run: no labs/*/*/check.sh found.\n'
fi

[ "$failed" -eq 0 ]
