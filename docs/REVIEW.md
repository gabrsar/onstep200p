# Public-import review

## Corrected and documented

- Replaced obsolete plugin docs with GPIO5/4/6 joystick and ALT10/9/8 mapping.
- Documented classic SC framing separately from numeric setters and checksums.
- Made dependencies explicit, checked the upstream pin and blocked local credentials in public builds.
- Imported only telescope source into new history. Private headers, images, raw logs and observing coordinates are excluded.
- Added CI, host tests, overlay checks, public-file audit and documentation validation.
- Added simulated previews with fictional data instead of publishing private display contents.

## Known limits

| Area | Limitation |
| --- | --- |
| Motion | Motors disconnected during validation; gearing and limits provisional |
| Clock | Tested Stellarium framing fixed; resync after boot |
| TCP | Single listener; short sessions complicate simultaneous diagnostics |
| App identity | Not transmitted by generic LX200 |
| Security | Plaintext, unauthenticated control and public development AP password |
| Overlays | Guarded text transformations; upstream upgrades need review |
| Previews | Representative fixtures, not live screenshots |
| CI | Software checks, no motor/on-sky validation |

This review does not claim exhaustive security coverage, universal LX200 compatibility or readiness for unattended motion.
