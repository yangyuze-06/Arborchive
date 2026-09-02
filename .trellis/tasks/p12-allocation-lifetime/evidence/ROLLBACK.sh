#!/bin/sh
set -eu
REPO=${1:-"$(git rev-parse --show-toplevel)"}
PATCH=${2:-"$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)/DIFF_FILE"}
cd "$REPO"
git apply -R --unidiff-zero --whitespace=nowarn "$PATCH"
printf 'ROLLBACK_RESULT=restored\n'
printf 'ROLLBACK_HEAD=%s\n' "$(git rev-parse HEAD)"
