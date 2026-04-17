# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

- Tooling: add a lightweight release workflow document and a helper script for safe version bumps.
- Documentation: clarify quartile convention used by the library.
  - The library uses the Tukey hinge quartile method and, by default, uses the
    exclusive-median variant (median excluded from lower/upper halves when computing
    Q1/Q3).
  - Small-sample exception: for sample size == 3 the implementation follows the
    common textbook convention and includes the median in both halves. This keeps
    Q1/Q3 values for three-element arrays consistent with textbook examples and
    existing unit tests.
