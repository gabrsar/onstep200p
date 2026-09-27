#!/usr/bin/env bash
set -euo pipefail

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "Erro: arduino-cli não encontrado. No macOS, instale com: brew install arduino-cli" >&2
  exit 1
fi

esp32_index="https://espressif.github.io/arduino-esp32/package_esp32_index.json"

arduino-cli core update-index --additional-urls "$esp32_index"
arduino-cli core install 'esp32:esp32@2.0.17' --additional-urls "$esp32_index"
arduino-cli lib install 'EspSoftwareSerial@8.1.0'

echo "Toolchain pronta: ESP32 core 2.0.17 + EspSoftwareSerial 8.1.0"
