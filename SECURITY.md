# Security policy

This is development firmware for physical equipment. Only the current main branch receives fixes. Do not assume this project supplies a safety-rated stop, collision protection or an authenticated network service.

LX200 over TCP is plaintext and unauthenticated. A device on the same network can send movement commands. The AP password in the source is a public development default. Keep the controller on a trusted network; never port-forward TCP 9999.

Report vulnerabilities through [GitHub private vulnerability reporting](https://github.com/gabrsar/onstep200p/security/advisories/new). Include a minimal reproduction and potential impact, but omit credentials, observing locations and private device logs. There is no guaranteed response time for this volunteer project.

`config/Wifi.local.h`, firmware images and build directories are ignored. Ignoring a file does not remove it from existing commits. If you disclose a real password, rotate it and contact the maintainer privately. Public builds fail when local Wi-Fi configuration is present; CI never receives home-network credentials.

Pull-request workflows use read-only permissions and do not run `pull_request_target`, access deployment secrets, auto-merge, or flash hardware. Action dependencies are pinned by commit and updated through Dependabot.
