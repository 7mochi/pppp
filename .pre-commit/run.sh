#!/usr/bin/env bash
set -euo pipefail

tool="$1"
shift

if command -v "$tool" >/dev/null 2>&1; then
  exec "$tool" "$@"
fi

if command -v nix >/dev/null 2>&1; then
  exec nix develop . --command "$tool" "$@"
fi

echo "$tool not found. Install it, or run the hooks inside the dev shell (nix develop)." >&2
exit 1
