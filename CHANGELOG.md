# Changelog

## Unreleased — public project setup

- Independent repository, curated documentation, protocol reference and hardware map.
- Host/protocol checks, public-file audit, ESP32 build matrix and pull-request templates.
- Reproducible OLED preview artwork with synthetic values.
- Public builds reject local Wi-Fi credentials; upstream revision checked before preparation.

## 0.1.6 — bench firmware baseline

- OLED pages, joystick calibration and persistent audio settings.
- Asynchronous AP/STA startup and a single LX200 TCP listener.
- Classic `SC` acknowledgment compatibility for the tested Stellarium client.
- Connection melody with unequal note spacing and delayed disconnection cue.

This baseline has not been validated with connected motors or on-sky tracking. The public setup does not imply a new hardware-qualified release.
