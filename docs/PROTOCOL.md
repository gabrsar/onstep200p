# Protocol reference and compatibility

OnStepX LX200 commands are available over native USB (configured115200 baud) and one standard TCP listener on9999. Persistent listeners9996–9998 are disabled. TCP is a byte stream: packet boundaries do not delimit replies.

The upstream standard server uses a short session timeout, so clients may reopen sockets. Sound coalesces reopens within5s. Use one client at a time; disconnect the app before separate diagnostics. A socket does not prove time readiness or alignment.

## Clock

`TIME_LOCATION_SOURCE OFF` means date/time must be resent after reboot. Stored location/timezone do not make the running clock ready.

| Command | Meaning | Reply |
| --- | --- | --- |
| `:SG+03:00#` | Offset added to local time to get UTC; example UTC−3 | Bare `1` or `0` |
| `:SL12:34:56#` | Set local time | Bare `1` or `0` |
| `:SC09/26/26#` | Set local date; four-digit year also accepted | `1` plus two `#`-terminated strings |
| `:GC#` / `:GL#` / `:GG#` | Date / time / timezone | Text ending in `#` |
| `:GX89#` | Clock readiness | Numeric `0` ready, `1` not ready |

Send timezone before local time/date. The successful classic SC bytes are:

```text
1Updating Planetary Data#                        #
```

Consume both strings. Invalid dates return bare `0`. Do not add a terminator to every setter. The internal broker and OnStep checksum extension retain their original single-value SC reply. The compatibility helper runs after execution and before transport framing; clock calculations are unchanged.

The tested Stellarium client previously timed out on SC despite the date being applied because upstream returned only `1`. Completing the classic strings fixed that observed failure. Other clients are not automatically certified. Leap years and ranges have host tests; the existing two-digit-year pivot is preserved, so custom clients should prefer four-digit years.

## Project queries

These experimental project extensions return text followed by `#`.

| Query | Result |
| --- | --- |
| `:GXPV#` | Version and application-image SHA-256 |
| `:GXPT#` | Date/time readiness, last SC/SL/SG values/results/channel |
| `:GXPC#` | TCP state, unknown app name, LX200 protocol |
| `:GXPW#` | HOME/AP/OFF and AP/STA flags |
| `:GXPN#` | STA/AP addresses; Wi-Fi builds only |
| `:GXPQ#` | Boot-stage timings and hash calculation time |
| `:GXPB#` | Sound state |
| `:GXPJ#` | Joystick input, center, switch, mode and state |
| `:GXPK#` | Persistent calibration and calibration stage |
| `:GXPA#` | First/settled ADC samples |
| `:GXPM#` | ADC readings and millivolts |

Sound setters return bare numeric success/failure: `:SXPB,0#` mutes, `:SXPB,1#` enables, `:SXPB,T#` tests boot audio and `:SXPB,E#` tests error audio without creating a mount error. Redact network addresses and private values from diagnostic logs.

## Review boundaries

Joystick commands use a FIFO-preserving local broker with stop enqueue retry. Review framing, numeric replies, checksums and the following command together. Generic `GR`/`GD` polling cannot identify Stellarium. `CM` and stopped slewing are controller events, not proof of physical alignment/arrival. Investigate timeouts before restarting away the evidence.

References: [pinned OnStepX](https://github.com/hjd1964/OnStepX/tree/65a751825677a03a0b12b15546780b614a206435), [Meade revision L](https://aggregate.org/DIT/CAPTURE/LX200CommandSet.pdf).
