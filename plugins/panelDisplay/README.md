# PanelDisplay

SSD1306 128×64 renderer on SDA18/SCL17, with differential I²C chunk updates,
boot diagnostics, six pages and live joystick indicators. Uses the internal
command broker for telemetry. Display data can lag hardware; unknown values
are not measurements. Addresses0x3C/0x3D are supported.

See [controls](../../docs/CONTROLS.md), [architecture](../../docs/ARCHITECTURE.md)
and [generated previews](../../docs/VISUALS.md).
