# Public development log

This log records public project changes. It contains no raw private device
session history, credentials, personal network identifiers or observing sites.

## 2026-09-27 — Independent repository

- Imported only the telescope source into a new repository and new Git history.
- Kept the upstream OnStepX GPL-3.0 submodule pinned at the reviewed revision.
- Replaced stale wiring documentation with the current GPIO map and bench status.
- Added English/Portuguese entry points, hardware/controls/protocol documentation,
  contribution/security policies, roadmap and attribution.
- Added original vector branding and reproducible synthetic OLED previews.
- Added default/minimal firmware CI, host tests, public-file/link checks,
  workflow lint and secret-history scanning with read-only PR permissions.
- No new firmware was uploaded to physical hardware during repository setup.

Verification and configuration:

- Local host regressions and the default/minimal embedded profiles compiled.
- Public-file audit, known-private-value comparison, Gitleaks history scan,
  Actionlint and local documentation checks passed.
- Initial GitHub CI passed all three checks: quality, default and minimal.
- Public repository uses a fresh history and a GitHub noreply commit address.
- Main requires PR/checks and resolved conversations; force pushes/deletion
  are blocked. Maintainer review is requested by CODEOWNERS, with no mandatory
  second reviewer for this single-maintainer project.
- Secret scanning, push protection, private vulnerability reports and
  Dependabot security updates are enabled. PR tokens remain read-only.
- PR2 removes a display-disabled compiler warning and updates checkout to a
  Node24 action. Arduino CLI is installed directly from its pinned release,
  with checksum verification, removing the old Node20 setup action.
- OLED preview artwork was inspected in a browser; fixtures are synthetic.
- Firmware on physical hardware was not changed by repository preparation.
