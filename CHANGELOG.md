# MontageForge

Every released version of MontageForge, newest first. A release publishes **one** section of this
file — the one whose heading matches its tag — as its release notes; for an `open` plugin those
notes are posted to Discord `#releases` automatically. Write for someone who installs the plugin,
not for the commit log.

Headings are `## <x.y.z> — <date>`. Use `Added` / `Changed` / `Fixed` / `Compatibility` /
`Known issues`, only the ones that apply.

## 0.1.3 — 2026-10-04

### Changed
- Copyright and licence notices now name Bojan Andrejek / MetaWorx LLC. It is still Apache 2.0, and nothing about how you may use it changed.

## 0.1.2 — 2026-09-08
- Packaging fix: the release now carries everything the register allows. `BuildPlugin`'s filter excludes `Config/` and every `public_extra` path, so earlier zips shipped without them.

## 0.1.1 — 2026-09-07
- Apache-2.0 licensing, and a release now publishes its source
- Every plugin descriptor agrees with its release tag, and says who made it
- Every Forge asset has a factory, and double-click opens the panel that owns it
- Every plugin now points at kovati.dev
- Tagged as MontageForge 0.1.1

## 0.1.0 — 2026-08-28
- Initial commit: Colony prototype, plugins and plans
- MontageForge: include AnimSequenceBase where the notify uses it
