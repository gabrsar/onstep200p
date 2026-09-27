#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
test_dir="$(mktemp -d /tmp/onstep200p-tests.XXXXXX)"
trap 'rm -f "$test_dir/test"; rmdir "$test_dir"' EXIT
for source in "$project_dir"/tests/*.cpp; do
  c++ -std=c++11 -Wall -Wextra -Werror "$source" -o "$test_dir/test"
  "$test_dir/test"
  echo "PASS $(basename "$source")"
done
broker_dir="$project_dir/.build/OnStepX/src/libApp/commands"
if [[ ! -f "$broker_dir/CommandBroker.cpp" ]]; then
  echo "Prepare source before testing the broker: scripts/prepare-source.sh" >&2
  exit 1
fi
# Compile the actual prepared broker, replacing only hardware dependencies.
{
  sed -n 'p' "$project_dir/tests/broker/stubs.h"
  sed '/^#include /d; /^#pragma once/d' "$broker_dir/CommandBroker.h"
  sed '/^#include /d' "$broker_dir/CommandBroker.cpp"
  sed -n 'p' "$project_dir/tests/broker/check.h"
} | c++ -std=c++11 -Wall -Wextra -Werror -x c++ - -o "$test_dir/test"
"$test_dir/test"
echo "PASS prepared CommandBroker FIFO and capacity"
# Verify overlays that move the intro ahead of hardware and remove only the
# generic service waits, retaining the telescope's driver settling delay.
prepared="$project_dir/.build/OnStepX"
rg -Uq 'void setup\(\) \{\n  panelFeedback.earlyBoot\(\);' "$prepared/OnStepX.ino"
rg -Fq 'tasks.yield(20);' "$prepared/OnStepX.ino"
rg -Fq 'panelFeedback.bootStage(4);' "$prepared/OnStepX.ino"
if rg -q 'delay\(1000\)' "$prepared/src/lib/serial/Serial_IP_Wifi.cpp"; then
  echo "FAIL TCP fixed startup wait remains" >&2; exit 1
fi
rg -Fq 'if (settings.stationEnabled && !settings.accessPointEnabled)' "$prepared/src/lib/wifi/WifiManager.cpp"
rg -Fq '  delay(1000);' "$prepared/src/telescope/Telescope.cpp"
echo "PASS early boot and nonblocking AP/STA overlays"
rg -q '^#define SERIAL_SERVER STANDARD$' "$prepared/Config.h"
echo "PASS single standard TCP listener configuration"
rg -Fq 'panelFeedbackTcpConnection(true)' "$prepared/src/lib/serial/Serial_IP_Wifi.cpp"
rg -Fq 'panelFeedbackTcpConnection(false)' "$prepared/src/lib/serial/Serial_IP_Wifi.cpp"
echo "PASS real TCP client notification hooks"
rg -Uq 'if \(!cmdSvrClient\) \{\n      if \(port == 9999\) panelFeedbackTcpConnection\(false\);' "$prepared/src/lib/serial/Serial_IP_Wifi.cpp"
echo "PASS remote-close notification before TCP reaccept"
rg -Fq 'lxDateClassicReply(buffer.getCmd()' "$prepared/src/libApp/commands/ProcessCmds.cpp"
echo "PASS classic LX200 date reply overlay"
