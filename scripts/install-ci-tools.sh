#!/usr/bin/env bash
set -euo pipefail

# Ephemeral Linux CI tools, pinned by version and verified against release
# checksums. Nothing is installed into the host package manager.
[[ "$(uname -s)" == Linux && "$(uname -m)" == x86_64 ]] || {
  echo "CI tool installer requires Linux x86_64." >&2; exit 2;
}
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
target="$project_dir/.build/ci-tools"
temp_dir="$(mktemp -d /tmp/onstep-ci-tools.XXXXXX)"
trap 'rm -f "$temp_dir/archive.tar.gz" "$temp_dir/checksums.txt"; rmdir "$temp_dir"' EXIT
mkdir -p "$target"

install_tool() {
  local repository="$1" version="$2" archive="$3" checksums="$4" binary="$5"
  local base="https://github.com/$repository/releases/download/v$version"
  curl --fail --location --silent --show-error "$base/$archive" -o "$temp_dir/archive.tar.gz"
  curl --fail --location --silent --show-error "$base/$checksums" -o "$temp_dir/checksums.txt"
  local expected
  expected="$(awk -v file="$archive" '$2 == file { print $1 }' "$temp_dir/checksums.txt")"
  [[ "$expected" =~ ^[0-9a-f]{64}$ ]] || { echo "Missing checksum for $binary" >&2; exit 1; }
  printf '%s  %s\n' "$expected" "$temp_dir/archive.tar.gz" | sha256sum --check --status
  tar -xzf "$temp_dir/archive.tar.gz" -C "$target" "$binary"
}

case "${1:-quality}" in
  quality)
    install_tool rhysd/actionlint 1.7.12 actionlint_1.7.12_linux_amd64.tar.gz actionlint_1.7.12_checksums.txt actionlint
    install_tool gitleaks/gitleaks 8.30.1 gitleaks_8.30.1_linux_x64.tar.gz gitleaks_8.30.1_checksums.txt gitleaks
    ;;
  firmware)
    install_tool arduino/arduino-cli 1.5.1 arduino-cli_1.5.1_Linux_64bit.tar.gz 1.5.1-checksums.txt arduino-cli
    ;;
  *) echo "Usage: $0 [quality|firmware]" >&2; exit 2;;
esac
