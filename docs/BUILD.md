# Build and verification

| Component | Pinned version |
| --- | --- |
| OnStepX | `65a751825677a03a0b12b15546780b614a206435` |
| Arduino CLI in CI | 1.5.1 |
| Arduino-ESP32 | 2.0.17 |
| EspSoftwareSerial | 8.1.0 |
| Board | ESP32-S3, 16 MB flash, 240 MHz, DIO, PSRAM disabled |

Install Git, Make, Bash, Perl, ripgrep, Python 3.10+ and a C++ compiler through your OS package manager. Install Arduino CLI following its official instructions. `make setup` installs the pinned core and library. The exact FQBN lives in `config/board.env`.

```sh
git submodule update --init --recursive
make setup
make quality
make check
```

Source is generated in `.build/OnStepX`. Output goes to `build/onstepx`, including `OnStepX.ino.bin` and its SHA-256 sidecar. The boot display and `:GXPV#` report the complete application image hash. Compiler timestamps mean independent builds need not have identical hashes: dependency inputs are pinned, not promised bit-for-bit reproducible.

## Profiles

```sh
make check SPEAKER=passive DISPLAY=on WIFI=on
make compile SPEAKER=off DISPLAY=off WIFI=off
make compile ALT_STEPS_PER_DEGREE=1234.5  # example only; measure your transmission
```

`passive` generates tones for an external driver circuit. `active` provides pulses for a buzzer with its own oscillator; it cannot reproduce pitches. `off` disables output. Make defaults to passive; invoking `scripts/compile.sh` directly defaults to off. Joystick processing remains compiled in both display profiles.

An empty ALT override uses a provisional bench value. AZ configuration and microstepping also need verification. Do not treat the example override as a recommendation.

## Private and public builds

Copy `config/Wifi.example.h` to `config/Wifi.local.h` and edit it for local station credentials. This file is ignored, but its contents enter your binary. Keep local builds private.

```sh
ONSTEP_PUBLIC_BUILD=1 make compile
```

Public builds refuse to proceed if `config/Wifi.local.h` exists. CI uses a fresh checkout, supplies no credentials and publishes no binary releases. Never upload a private local build as a release artifact.

## Upload and CI

```sh
make list
make install PORT=/dev/ttyACM0
```

macOS can auto-detect a single `/dev/cu.usbmodem*` device. Otherwise provide a port. The upload guard requires a completed build; changing Make variables at upload time does not rebuild it. Uploading resets the controller. Calibration and mute normally remain in NVS; clock synchronization must be resent. `make clean` removes only this checkout's generated source and application output.

Ordinary PRs run quality/host tests plus default and minimal ESP32 builds. Workflows use read-only permissions, pinned actions and no deployment secrets. Software checks do not prove electrical or mechanical safety.
