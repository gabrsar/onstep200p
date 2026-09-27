#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Uso: $0 [--alt-steps-per-degree NUMERO] [--display off|on] [--wifi off|on] [--speaker off|active|passive]" >&2
}

alt_steps="1750.42735"
provisional_altitude=true
display_mode="on"
wifi_mode="on"
speaker_mode=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --speaker)
      case "${2:-}" in
        off) speaker_mode=0;; active) speaker_mode=1;; passive) speaker_mode=2;;
        *) usage; exit 2;;
      esac
      shift 2
      ;;
    --alt-steps-per-degree)
      [[ $# -ge 2 && -n "$2" ]] || { usage; exit 2; }
      alt_steps="$2"
      provisional_altitude=false
      shift 2
      ;;
    --display)
      [[ $# -ge 2 && ( "$2" == "off" || "$2" == "on" ) ]] || { usage; exit 2; }
      display_mode="$2"
      shift 2
      ;;
    --wifi)
      [[ $# -ge 2 && ( "$2" == "off" || "$2" == "on" ) ]] || { usage; exit 2; }
      wifi_mode="$2"
      shift 2
      ;;
    *)
      usage
      exit 2
      ;;
  esac
done

if ! awk -v value="$alt_steps" 'BEGIN { exit !(value ~ /^[0-9]+([.][0-9]+)?$/ && value > 0) }'; then
  echo "Erro: steps/degree da altitude deve ser um numero positivo." >&2
  exit 2
fi

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
upstream_dir="$project_dir/vendor/OnStepX"
build_root="$project_dir/.build"
build_dir="$build_root/OnStepX"

if [[ "${ONSTEP_PUBLIC_BUILD:-0}" == "1" && -f "$project_dir/config/Wifi.local.h" ]]; then
  echo "Erro: public build refuses local Wi-Fi credentials." >&2
  exit 1
fi

if [[ ! -f "$upstream_dir/OnStepX.ino" ]]; then
  echo "Erro: submodulo ausente. Rode: git submodule update --init --recursive" >&2
  exit 1
fi
upstream_revision="$(git -C "$upstream_dir" rev-parse HEAD)"
if [[ "$upstream_revision" != "65a751825677a03a0b12b15546780b614a206435" ]]; then
  echo "Erro: upstream revision differs from the reviewed project pin." >&2
  exit 1
fi

mkdir -p "$build_root"
if [[ -e "$build_dir" ]]; then
  rm -rf "$build_dir"
fi
cp -R "$upstream_dir" "$build_dir"
rm -rf "$build_dir/.git"

# Private station credentials never enter Git. Apply after NV loading so an
# existing AP-only configuration cannot silently override this local profile.
if [[ -f "$project_dir/config/Wifi.local.h" && "$wifi_mode" == "on" ]]; then
  cp "$project_dir/config/Wifi.local.h" "$build_dir/Wifi.local.h"
  perl -0pi -e 's/\A/#include "Wifi.local.h"\n/' "$build_dir/Config.h"
  perl -0pi -e 's/  #if DEBUG != OFF/  #ifdef ONSTEP200P_LOCAL_WIFI\n    settings.accessPointEnabled = true;\n    settings.stationEnabled = true;\n    stationNumber = 1;\n    station[0].dhcpEnabled = true;\n    snprintf(station[0].ssid, sizeof(station[0].ssid), "%s", STA_SSID);\n    snprintf(stationPassword[0].password, sizeof(stationPassword[0].password), "%s", STA_PASSWORD);\n  #endif\n\n  #if DEBUG != OFF/' "$build_dir/src/lib/wifi/WifiManager.cpp"
fi
# Upstream verbose logging includes passwords; redact even in debug builds.
perl -pi -e 's/V\(settings.ap.pwd\)/V("[redacted]")/g; s/VL\(settings.ap.pwd\)/VL("[redacted]")/g; s/VL\(staPwd->password\)/VL("[redacted]")/g; s/VL\(settings.masterPassword\)/VL("[redacted]")/g' "$build_dir/src/lib/wifi/WifiManager.cpp"

# The pinned broker scans slots by index, which can reorder a newly inserted
# rate/stop command ahead of an older pending motion command. Preserve FIFO.
perl -0pi -e 's/bool replyExpected = true;/bool replyExpected = true;\n      uint32_t sequence = 0;/; s/Request requests\[COMMAND_BROKER_SLOTS\];/Request requests[COMMAND_BROKER_SLOTS];\n    uint32_t nextSequence = 0;/' "$build_dir/src/libApp/commands/CommandBroker.h"
perl -0pi -e 's/requests\[i\].state = CB_PENDING;/requests[i].sequence = nextSequence++;\n      requests[i].state = CB_PENDING;/; s/for \(uint8_t i = 0; i < COMMAND_BROKER_SLOTS; i\+\+\) \{\n      if \(requests\[i\].state == CB_PENDING\) \{.*?\n    \}/int8_t oldest = -1;\n    for (uint8_t i = 0; i < COMMAND_BROKER_SLOTS; i++) {\n      if (requests[i].state == CB_PENDING \&\&\n          (oldest < 0 || (int32_t)(requests[i].sequence - requests[oldest].sequence) < 0)) oldest = i;\n    }\n    if (oldest >= 0) {\n      if (requests[oldest].replyExpected) requests[oldest].deadline = millis() + requests[oldest].timeoutMs;\n      requests[oldest].state = CB_SENT;\n      active = oldest;\n    }/s' "$build_dir/src/libApp/commands/CommandBroker.cpp"
rg -q 'active = oldest;' "$build_dir/src/libApp/commands/CommandBroker.cpp" || { echo "Erro: overlay FIFO ausente." >&2; exit 1; }

cp "$project_dir/config/LxDate.h" "$build_dir/src/telescope/mount/site/LxDate.h"
perl -0pi -e 's/#include "Site.h"/#include "Site.h"\n#include "LxDate.h"/; s/bool Site::strToDate\(char \*ymd, GregorianDate \*date\) \{.*?\n\}/bool Site::strToDate(char *ymd, GregorianDate *date) {\n  LxDate parsed;\n  if (!parseLxDate(ymd, parsed)) return false;\n  date->year = parsed.year;\n  date->month = parsed.month;\n  date->day = parsed.day;\n  return true;\n}/s' "$build_dir/src/telescope/mount/site/Site.cpp"
rg -Fq 'parseLxDate(ymd, parsed)' "$build_dir/src/telescope/mount/site/Site.cpp" || { echo "Erro: overlay de data ausente." >&2; exit 1; }
cp "$project_dir/config/LxDateReply.h" "$build_dir/src/libApp/commands/LxDateReply.h"
perl -0pi -e 's/\A/#include "LxDateReply.h"\n/; s/(    if \(strlen\(reply\) > 0 \|\| buffer.checksum\) \{)/    lxDateClassicReply(buffer.getCmd(), commandError == CE_NONE || commandError == CE_1,\n      buffer.checksum, channel, reply, replyCapacity, suppressFrame);\n$1/' "$build_dir/src/libApp/commands/ProcessCmds.cpp"
rg -Fq 'lxDateClassicReply(buffer.getCmd()' "$build_dir/src/libApp/commands/ProcessCmds.cpp" || { echo "Erro: resposta SC clássica ausente." >&2; exit 1; }

perl -pi -e 's/#define HOST_NAME\s+"OnStep"/#define HOST_NAME             "OnStep200P"/' "$build_dir/Config.h"
perl -0pi -e "s/(#define HOST_NAME[^\n]*\n)/\$1#define ONSTEP200P_SPEAKER $speaker_mode\n#define ONSTEP200P_SPEAKER_PIN 7\n/" "$build_dir/Config.h"
perl -0pi -e 's/(#define HOST_NAME[^\n]*\n)/$1#define AP_SSID               "OnStep200P"\n#define AP_PASSWORD           "onstepx200p"\n/' "$build_dir/Config.h"
if [[ "$display_mode" == "on" ]]; then
  perl -0pi -e 's/(#define AP_PASSWORD[^\n]*\n)/$1#define ONSTEP200P_DISPLAY     ON\n/' "$build_dir/Config.h"
else
  perl -0pi -e 's/(#define AP_PASSWORD[^\n]*\n)/$1#define ONSTEP200P_DISPLAY     OFF\n/' "$build_dir/Config.h"
  perl -pi -e 's/^  WIRE_INIT\(\);/  \/\/ WIRE_INIT skipped: safe profile for the current OLED\/I2C fault./' "$build_dir/OnStepX.ino"
fi
perl -pi -e 's/#define PINMAP\s+OFF/#define PINMAP              OnStep200PS3/' "$build_dir/Config.h"
perl -pi -e 's/#define SERIAL_A_BAUD_DEFAULT\s+9600/#define SERIAL_A_BAUD_DEFAULT      115200/' "$build_dir/Config.h"
# One telescope TCP listener. Change STANDARD to BOTH here to restore
# persistent listeners 9996-9998 alongside the standard port 9999.
perl -0pi -e 's/(#define SERIAL_A_BAUD_DEFAULT[^\n]*\n)/$1#define SERIAL_SERVER STANDARD\n/' "$build_dir/Config.h"
perl -pi -e 's/#define SERIAL_B_BAUD_DEFAULT\s+9600/#define SERIAL_B_BAUD_DEFAULT         OFF/' "$build_dir/Config.h"
if [[ "$wifi_mode" == "on" ]]; then
  perl -pi -e 's/#define SERIAL_RADIO\s+OFF/#define SERIAL_RADIO    WIFI_ACCESS_POINT/' "$build_dir/Config.h"
fi
perl -pi -e 's/#define AXIS1_DRIVER_MODEL\s+OFF/#define AXIS1_DRIVER_MODEL       TMC2209S/' "$build_dir/Config.h"
perl -pi -e 's/#define AXIS1_STEPS_PER_DEGREE\s+12800/#define AXIS1_STEPS_PER_DEGREE 1750.42735/' "$build_dir/Config.h"
perl -pi -e 's/#define AXIS1_DRIVER_MICROSTEPS\s+OFF/#define AXIS1_DRIVER_MICROSTEPS        64/' "$build_dir/Config.h"
perl -pi -e 's/#define AXIS2_DRIVER_MODEL\s+OFF/#define AXIS2_DRIVER_MODEL       TMC2209S/' "$build_dir/Config.h"
perl -pi -e 's/#define AXIS2_STEPS_PER_DEGREE\s+12800/#define AXIS2_STEPS_PER_DEGREE \@ALT_STEPS_PER_DEGREE\@/' "$build_dir/Config.h"
perl -pi -e 's/#define AXIS2_LIMIT_MIN\s+-90/#define AXIS2_LIMIT_MIN                 0/' "$build_dir/Config.h"
perl -pi -e 's/#define AXIS2_DRIVER_MICROSTEPS\s+OFF/#define AXIS2_DRIVER_MICROSTEPS        64/' "$build_dir/Config.h"
perl -pi -e 's/#define MOUNT_TYPE\s+GEM/#define MOUNT_TYPE                 ALTAZM/' "$build_dir/Config.h"
perl -pi -e 's/#define MOUNT_STARTUP_MODE\s+SA_AUTO/#define MOUNT_STARTUP_MODE      SA_STRICT/' "$build_dir/Config.h"
perl -pi -e 's/#define SLEW_RATE_BASE_DESIRED\s+1\.0/#define SLEW_RATE_BASE_DESIRED        0.5/' "$build_dir/Config.h"

perl -0pi -e 's/#define PINMAP_LAST\s+24/#define OnStep200PS3                25     \/\/ OnStep200P ESP32-S3 wiring profile\n#define PINMAP_LAST                 25/' "$build_dir/src/Constants.h"
perl -0pi -e 's/(#if PINMAP == SAL_XB1.*?#endif\n)/$1#if PINMAP == OnStep200PS3\n  #define PINMAP_STR "OnStep200P ESP32-S3"\n  #include "Pins.OnStep200PS3.h"\n#endif\n/s' "$build_dir/src/pinmaps/Models.h"
cp "$project_dir/config/Pins.OnStep200PS3.h" "$build_dir/src/pinmaps/Pins.OnStep200PS3.h"

if [[ "$display_mode" == "on" ]]; then
  perl -pi -e 's/#define PLUGIN1\s+OFF/#define PLUGIN1              panelDisplay/' "$build_dir/src/plugins/Plugins.config.h"
  perl -0pi -e 's/(^#define PLUGIN1[^\n]*\n)/$1#include "panelDisplay\/PanelDisplay.h"\n/m' "$build_dir/src/plugins/Plugins.config.h"
  cp -R "$project_dir/plugins/panelDisplay" "$build_dir/src/plugins/panelDisplay"
  # Early OLED diagnostics, after Wire exists but before the command broker.
  perl -0pi -e 's/(  WIRE_INIT\(\);)/$1\n  panelDisplay.boot("I2C OK - STORAGE");/; s/(  telescope.init\()/  panelDisplay.boot("MOUNT AND WIFI");\n$1/; s/(  commandChannelInit\(\);)/  panelDisplay.boot("COMMAND CHANNELS");\n$1\n  panelDisplay.boot("STARTING SERVICES");/' "$build_dir/OnStepX.ino"
  rg -q 'panelDisplay.boot\("STARTING SERVICES"\)' "$build_dir/OnStepX.ino" || { echo "Erro: hook de boot ausente." >&2; exit 1; }
  # Observe completed command frames; never claim an application identity.
  perl -0pi -e 's/if \(buffer.ready\(\)\) \{/if (buffer.ready()) {\n    extern void panelDisplayObserveChannel(char);\n    panelDisplayObserveChannel(channel);/' "$build_dir/src/libApp/commands/ProcessCmds.cpp"
  rg -q 'panelDisplayObserveChannel\(channel\)' "$build_dir/src/libApp/commands/ProcessCmds.cpp" || { echo "Erro: hook de origem dos comandos não aplicado." >&2; exit 1; }
fi
perl -pi -e 's/#define PLUGIN2\s+OFF/#define PLUGIN2             panelJoystick/' "$build_dir/src/plugins/Plugins.config.h"
perl -0pi -e 's/(^#define PLUGIN2[^\n]*\n)/$1#include "panelJoystick\/PanelJoystick.h"\n/m' "$build_dir/src/plugins/Plugins.config.h"
cp -R "$project_dir/plugins/panelJoystick" "$build_dir/src/plugins/panelJoystick"
perl -pi -e 's/#define PLUGIN3\s+OFF/#define PLUGIN3 panelFeedback/; s/#define PLUGIN3_COMMAND_PROCESSING\s+OFF/#define PLUGIN3_COMMAND_PROCESSING ON/' "$build_dir/src/plugins/Plugins.config.h"
perl -0pi -e 's/(^#define PLUGIN3[^\n]*\n)/$1#include "panelFeedback\/PanelFeedback.h"\n/m' "$build_dir/src/plugins/Plugins.config.h"
cp -R "$project_dir/plugins/panelFeedback" "$build_dir/src/plugins/panelFeedback"
# Play the intro before HAL/NV/Wi-Fi setup. Keep driver settling delays in
# Telescope/Mount, but remove fixed waits around synchronous storage/servers.
perl -0pi -e 's/void setup\(\) \{/void setup() {\n  panelFeedback.earlyBoot();/; s/(  HAL_INIT\(\);)/$1\n  panelFeedback.bootStage(0);/; s/  delay\(2000\);\n\n  #if defined\(NV_WIPE\)/  panelFeedback.bootStage(1);\n\n  #if defined(NV_WIPE)/; s/(  telescope.init\([^\n]+;)/$1\n  panelFeedback.bootStage(2);/; s/  tasks.yield\(2000\);/  tasks.yield(20);\n  panelFeedback.bootStage(3);/; s/(  telescope.ready = true;)/$1\n  panelFeedback.bootStage(4);/' "$build_dir/OnStepX.ino"
perl -pi -e 's/^    delay\(1000\);/    \/\/ WiFiServer begin is synchronous; no fixed startup delay./' "$build_dir/src/lib/serial/Serial_IP_Wifi.cpp"
# Report actual TCP accept/close, including clients arriving via home Wi-Fi.
perl -0pi -e 's/(#include "Serial_IP_Wifi.h")/$1\nextern void panelFeedbackTcpConnection(bool connected);/; s/(        cmdSvrClient = cmdSvr->available\(\);)/$1\n        if (port == 9999 \&\& cmdSvrClient) panelFeedbackTcpConnection(true);/; s/(\s*)cmdSvrClient.stop\(\);/$1if (port == 9999) panelFeedbackTcpConnection(false);$1cmdSvrClient.stop();/g' "$build_dir/src/lib/serial/Serial_IP_Wifi.cpp"
rg -Fq 'panelFeedbackTcpConnection(true)' "$build_dir/src/lib/serial/Serial_IP_Wifi.cpp" || { echo "Erro: hook TCP ausente." >&2; exit 1; }
# WiFiClient::operator bool reports connected(), so a remote close takes
# the no-client branch rather than the explicit stop branch below it.
perl -0pi -e 's/(    if \(!cmdSvrClient\) \{)/$1\n      if (port == 9999) panelFeedbackTcpConnection(false);/' "$build_dir/src/lib/serial/Serial_IP_Wifi.cpp"
perl -pi -e 's/\{ delay\(200\); SerialPort.begin/\{ SerialPort.begin/' "$build_dir/src/libApp/commands/ProcessCmds.cpp"
# With our AP already enabled, the home Wi-Fi association can finish in the
# background; all TCP listeners bind without waiting up to eight seconds.
perl -0pi -e 's/(    \/\/ wait for connection\n    if \(settings.stationEnabled)\)/$1 \&\& !settings.accessPointEnabled)/' "$build_dir/src/lib/wifi/WifiManager.cpp"
rg -Fq 'panelFeedback.earlyBoot();' "$build_dir/OnStepX.ino" || { echo "Erro: hook de boot inicial ausente." >&2; exit 1; }
rg -Fq 'panelFeedback.bootStage(4);' "$build_dir/OnStepX.ino" || { echo "Erro: marcador de boot completo ausente." >&2; exit 1; }
rg -Fq 'if (settings.stationEnabled && !settings.accessPointEnabled)' "$build_dir/src/lib/wifi/WifiManager.cpp" || { echo "Erro: overlay de STA assíncrona ausente." >&2; exit 1; }
# Preserve parameters before timezone parsing mutates its input, then observe
# successful/failed clock writes without printing into the LX200 USB stream.
perl -0pi -e 's/(    commandError = command\(reply, buffer.getCmd\(\), buffer.getParameter\(\), &suppressFrame, &numericReply\);)/    char panelParameter[40];\n    snprintf(panelParameter, sizeof(panelParameter), "%s", buffer.getParameter());\n$1\n    extern void panelFeedbackObserveCommand(char, const char*, const char*, int, const char*);\n    panelFeedbackObserveCommand(channel, buffer.getCmd(), panelParameter, commandError, reply);/' "$build_dir/src/libApp/commands/ProcessCmds.cpp"
rg -Fq 'panelFeedbackObserveCommand(channel' "$build_dir/src/libApp/commands/ProcessCmds.cpp" || { echo "Erro: hook feedback ausente." >&2; exit 1; }

escaped_alt_steps="${alt_steps//\//\\/}"
sed "s/@ALT_STEPS_PER_DEGREE@/$escaped_alt_steps/" "$build_dir/Config.h" > "$build_dir/Config.h.tmp"
mv "$build_dir/Config.h.tmp" "$build_dir/Config.h"

if rg -q '@ALT_STEPS_PER_DEGREE@' "$build_dir/Config.h"; then
  echo "Erro: placeholder da altitude nao foi substituido." >&2
  exit 1
fi

required_markers=(
  'PINMAP              OnStep200PS3'
  'AXIS1_DRIVER_MODEL       TMC2209S'
  'AXIS2_DRIVER_MODEL       TMC2209S'
  'MOUNT_TYPE                 ALTAZM'
  'PINMAP_STR "OnStep200P ESP32-S3"'
  '#include "panelJoystick/PanelJoystick.h"'
)
for marker in "${required_markers[@]}"; do
  if ! rg --no-ignore -F -q "$marker" "$build_dir"; then
    echo "Erro: a revisão upstream mudou e o overlay não encontrou: $marker" >&2
    exit 1
  fi
done
if [[ "$display_mode" == "on" ]]; then
  if ! rg --no-ignore -q '^#define PLUGIN1\s+panelDisplay' "$build_dir/src/plugins/Plugins.config.h"; then
    echo "Erro: o plugin PanelDisplay não foi ativado." >&2
    exit 1
  fi
elif ! rg --no-ignore -q '^#define PLUGIN1\s+OFF' "$build_dir/src/plugins/Plugins.config.h"; then
  echo "Erro: o perfil solicitado deveria manter o plugin de display desligado." >&2
  exit 1
fi
if [[ "$display_mode" == "off" ]] && rg -q '^  WIRE_INIT\(\);' "$build_dir/OnStepX.ino"; then
  echo "Erro: o perfil seguro não desativou a inicialização I2C." >&2
  exit 1
fi
if ! rg --no-ignore -q '^#define PLUGIN2\s+panelJoystick' "$build_dir/src/plugins/Plugins.config.h"; then
  echo "Erro: o plugin PanelJoystick não foi ativado." >&2
  exit 1
fi
if [[ "$wifi_mode" == "on" ]]; then
  if ! rg -q '^#define SERIAL_RADIO\s+WIFI_ACCESS_POINT' "$build_dir/Config.h"; then
    echo "Erro: o perfil Wi-Fi não ativou o access point." >&2
    exit 1
  fi
elif ! rg -q '^#define SERIAL_RADIO\s+OFF' "$build_dir/Config.h"; then
  echo "Erro: o perfil solicitado deveria manter o rádio Wi-Fi desligado." >&2
  exit 1
fi

echo "Fonte preparada em: $build_dir"
echo "Altitude configurada em: $alt_steps steps/degree"
echo "Display/I2C: $display_mode"
echo "Wi-Fi: $wifi_mode"
if [[ "$display_mode" == "off" ]]; then
  echo "AVISO: OLED e inicialização I2C desativados por solicitação."
fi
if [[ "$wifi_mode" == "off" ]]; then
  echo "AVISO: rádio Wi-Fi desativado por solicitação."
fi
if [[ "$provisional_altitude" == true ]]; then
  echo "AVISO: redução da altitude provisória; não usar com motores conectados."
fi
