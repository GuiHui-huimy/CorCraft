#!/usr/bin/env bash
# Acceptance check for LAB cpp_13_capstone.
# Builds that one target with the project flags, then runs it: the exit status
# is the result, because a lab that prints an error and continues is not a lab
# that passed.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
build="${root}/build-labs"

log="$(mktemp)"
trap 'rm -f "${log}"' EXIT

cmake -S "${root}/code" -B "${build}" -DCMAKE_BUILD_TYPE=Debug > "${log}" 2>&1
cmake --build "${build}" --target cpp_13_capstone -j >> "${log}" 2>&1

if grep -q 'warning:' "${log}"; then
  echo "the build produced warnings; the book's rule is zero:" >&2
  grep 'warning:' "${log}" >&2
fi

"${build}/cpp_13_capstone"
