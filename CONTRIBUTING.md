# Contributing

Clone with submodules and read [the architecture](docs/ARCHITECTURE.md). Keep changes small enough to review and describe the behavior before and after the change.

## Local checks

```sh
make quality     # Public-file audit, documentation links, generated previews
make test        # C++ logic tests and upstream overlay assertions
make check       # Tests plus ESP32-S3 compilation
```

Do not edit `vendor/OnStepX` or `.build/OnStepX` in place. Update the tracked overlay/config/plugin that generates the source. An upstream version change must update the submodule pin and overlay tests together. Use Python 3.10+ for repository tooling.

For protocol changes, document framing and error responses in `docs/PROTOCOL.md`. Tests should exercise observable behavior, including the next command after a response. Preserve the `:Q#` stop path, FIFO order, nonblocking feedback, mute, calibration persistence and numeric replies used by the local broker.

For UI changes, regenerate previews with `make previews`. Label fixtures as simulated; never include real passwords, precise observing coordinates or private device identifiers. Record hardware tests honestly: a successful compile is not a tracking test.

## Pull requests and review

CI runs on pushes and ordinary pull requests. It checks source hygiene, host tests, protocol guards, and default/minimal ESP32 builds. CODEOWNERS requests maintainer review; the PR checklist focuses review on framing, pin changes and motor behavior. CI does not provide autonomous human-equivalent or AI code review.

Use a focused branch and include test results, affected hardware and any migration requirements. Do not commit Wi-Fi credentials, firmware produced with private configuration, serial logs, generated build trees or unrelated project files. Do not attach an unrestricted filesystem dump.

No contribution agreement is required; contributions are submitted under the repository's GPL-3.0 license. Be considerate, discuss code rather than people, and respect maintainers' time.
