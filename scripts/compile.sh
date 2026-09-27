#!/usr/bin/env bash
set -euo pipefail

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "Erro: arduino-cli não encontrado. Rode scripts/setup-toolchain.sh primeiro." >&2
  exit 1
fi

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$project_dir/config/board.env"
output_dir="$project_dir/build/onstepx"
mkdir -p "$output_dir"
rm -f "$output_dir/.compile-ok"
"$project_dir/scripts/prepare-source.sh" "$@"

cli_flags=()
[[ "${VERBOSE:-0}" != "1" ]] || cli_flags+=(--verbose)
arduino-cli compile "${cli_flags[@]}" \
  --fqbn "$ONSTEPX_FQBN" \
  --warnings all \
  --output-dir "$output_dir" \
  "$project_dir/.build/OnStepX"
(
  cd "$output_dir"
  shasum -a 256 OnStepX.ino.bin > OnStepX.ino.bin.sha256
)
touch "$output_dir/.compile-ok"

echo "Binários gerados em: $output_dir"
echo "Nenhum firmware foi gravado na placa."
