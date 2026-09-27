# Display, joystick and sound

The SSD1306 is monochrome. Pages are selected manually. The header shows UI/mount mode, click state, an eight-direction arrow with strength and input state.

| Page | Contents |
| --- | --- |
| MOUNT | AZ/ALT, tracking/GoTo status |
| OBSERVATORY | Clock readiness, date, coordinates, elevation |
| NETWORK | AP/STA, IP, TCP9999, connected LX200 socket, scrolling SSID |
| ACCESS | AP credentials; avoid photographing real passwords |
| SYSTEM | Command source, sound, timezone, initialization error |
| INPUT | Raw input and calibration |

Generic LX200 polling does not identify an app or human-readable target name. `APP LX200 CONNECTED` means a TCP socket is present, including diagnostic tools.

## Gestures and calibration

Pressing the switch requests an immediate stop. One short click advances a page after the450ms multi-click window; three short clicks toggle UI/mount mode. UI navigation requires direction settling and recentring, so holding a direction does not run a carousel.

Hold1.5–5s and release to toggle persistent mute. Hold at least5s to calibrate; calibration blocks joystick motion commands and retains stop behavior.

1. Hear the entry beep, release the switch and center for the initial reference.
2. Move around the full rim three times, including diagonals. Back-and-forth motion does not count as a revolution.
3. After the beep, click once, release and rest a light finger at center for3s.
4. Center/noise are measured, dead zones expanded by10%, and the profile saved to NVS. Two beeps confirm a successful save.

Each half-axis is scaled independently. Leaving center during capture restarts its timer. Invalid travel is rejected; save failures retry without reporting success. Five minutes cancels unfinished calibration. Recalibrate after wiring/extension changes.

## Sound

Boot plays the opening of “Shave and a Haircut”; ready completes the last two notes after initialization. Home Wi-Fi uses three ascending notes; AP fallback uses two lower notes after a grace period. App connection uses C♯–C♯–B–C♯–E–C♯ with communicator-style timing. A descending cue marks five continuous seconds offline; quick reopens do not repeat alerts.

Error tones have priority. Calibration has entry/stage/save cues and an input-related tone during the rim sweep. All sounds honor mute and use nonblocking scheduling. GoTo completion reports that slewing ended, not optical confirmation of target acquisition.
