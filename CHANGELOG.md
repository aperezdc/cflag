# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Support building in C23 mode (e.g. `-std=c23`).

## [1.1.1]
### Added
- Line-wrap and indent option help texts, by default to 80 columns, or to
  the actual terminal size in systems where `TIOCGWINSZ` is available.

## [1.1.0] - 2020-08-19
### Added
- Support `float` and `double` flags.

## [1.0.0] - 2020-08-18
### Changed
- Remove `typedef` for types `CFlag` and `CFlagStatus`, instead user code
  must use `struct cflag` and `enum cflag_status` directly.

## [0.1.1] - 2020-06-05
### Added
- Support `unsigned int` flags.
- Support file size (`size_t`) flags with SI suffixes.
- Support time interval (`unsigned long long`) flags with conventional time
  suffixes (e.g. `2d` for “two days”).

## [0.1.0] - 2020-06-01

Initial release.

[Unreleased]: https://github.com/aperezdc/cflag/compare/1.1.2...HEAD
[1.1.2]: https://github.com/aperezdc/cflag/compare/1.1.1...1.1.2
[1.1.1]: https://github.com/aperezdc/cflag/compare/1.1.0...1.1.1
[1.1.0]: https://github.com/aperezdc/cflag/compare/1.0.0...1.1.0
[1.0.0]: https://github.com/aperezdc/cflag/compare/0.1.1...1.0.0
[0.1.1]: https://github.com/aperezdc/cflag/compare/0.1.0...0.1.1
[0.1.0]: https://github.com/aperezdc/cflag/releases/tag/0.1.0
