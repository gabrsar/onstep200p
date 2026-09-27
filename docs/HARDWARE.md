# Hardware and commissioning

The bench profile targets an ESP32-S3 N16R8, SSD1306 128×64 I²C OLED, HW-504 analog joystick and two TMC2209 drivers in standalone STEP/DIR mode. Motor gearing/current and physical driver-to-axis assignment still need verification.

| Function | ESP32-S3 GPIO | Notes |
| --- | ---: | --- |
| AZ enable / step / direction | 1 / 2 / 42 | Axis 1, physical assignment unverified |
| ALT enable / step / direction | 10 / 9 / 8 | Axis 2; avoid OPI memory pins |
| OLED SDA / SCL | 18 / 17 | SSD1306, `0x3C` or `0x3D` |
| Joystick VRx / VRy / SW | 5 / 4 / 6 | Powered from 3.3 V |
| Speaker control | 7 | External transistor driver |
| USB | Native USB-C | USB CDC/JTAG |

`config/Pins.OnStep200PS3.h` is the source of truth. GPIO35–37 are tied to octal memory on S3R8 variants; disabling PSRAM does not make those connections suitable for external signals. Verify your particular board.

The joystick is mounted90° rotated: VRx is vertical and VRy horizontal. Feed GPIO inputs at3.3V even if the module says “5V”. Bad extensions/connectors can produce coupled readings; compare voltage at the GPIO and module before compensating in software.

## Speaker circuit used on the bench

A roughly45–50Ω magnetic PC speaker was driven by a BC547B:

```text
GPIO7 -- 1 kΩ -- base
                  | 10 kΩ
GND --------------+-------- emitter
5 V -- 100 Ω -- speaker (+)
                  speaker (-) -- collector
```

Place a flyback diode across the speaker, cathode/stripe at (+), anode at (−). Confirm transistor pin order from its datasheet. The100Ω series resistor used was at least¼W; all grounds are common. This describes the tested circuit, not a universal design for arbitrary speakers. Never drive this load directly from a GPIO.

## Commissioning

Bench electronics used5V supply and3.3V logic. Motor torque and slew rate on that supply have not been validated. Check driver current/cooling, coil pairs and microstepping; never change motor wiring with drivers energized. Vcc is not measured without a suitable sensor circuit.

1. Verify the pin map, especially ALT10/9/8, and identify each axis.
2. Measure transmission ratios and configure steps/degree and microstepping.
3. Secure the mechanism, provide a physical way to cut motor power and test at low speed.
4. Verify directions, stop action, travel limits and cable clearance.
5. Then validate alignment, GoTo and tracking on the sky.

The configured ALT0–90° software limits do not guarantee collision prevention. None of these powered-axis tests is claimed complete by CI.
