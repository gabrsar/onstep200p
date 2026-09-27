#!/usr/bin/env bash
set -euo pipefail

port="${1:?Informe explicitamente a porta USB atual}"
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$project_dir/config/board.env"

if [[ ! -c "$port" ]]; then
  echo "Erro: porta serial não encontrada: $port" >&2
  exit 1
fi
if [[ ! -f "$project_dir/build/onstepx/OnStepX.ino.bin" || ! -f "$project_dir/build/onstepx/.compile-ok" ]]; then
  echo "Erro: firmware ausente. Rode scripts/compile.sh primeiro." >&2
  exit 1
fi

echo "Porta: $port"
echo "AVISO: use somente com os motores desconectados enquanto a altitude for provisória."
cli_flags=()
[[ "${VERBOSE:-0}" != "1" ]] || cli_flags+=(--verbose)
arduino-cli upload "${cli_flags[@]}" \
  --fqbn "$ONSTEPX_FQBN" \
  --port "$port" \
  --input-dir "$project_dir/build/onstepx" \
  "$project_dir/.build/OnStepX"
