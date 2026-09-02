#!/usr/bin/env bash
set -euo pipefail
ROOT="$(git rev-parse --show-toplevel)"
TARGET="${1:?usage: ROLLBACK.sh TARGET_COPY}"
git -C "$ROOT" show dbd9972:src/core/processor/expr_processor.cc > "$TARGET"
sha="$(shasum -a 256 "$TARGET" | awk '{print $1}')"
behavior="$(grep -q 'return loadExprId' "$TARGET" && printf 'pre-fix-load-chain' || printf 'unexpected')"
printf 'ROLLBACK_RESTORED sha=%s behavior=%s\n' "$sha" "$behavior"
