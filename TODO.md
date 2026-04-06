# TODO

## Documentation

- Decide whether generated docs should be published as a GitHub Pages site, a CI artifact, or both.
- Expand Doxygen coverage for the public API surface, especially around error-returning functions and quartile conventions.
- Add a short contributor note describing the preferred Doxygen style for templates, concepts, and inline utility aliases.

## Tooling

- Keep local WSL, Docker, and CI Clang toolchains aligned to reduce `clang-tidy` drift.
- Consider running documentation generation inside the same Docker image used for Linux Clang CI so doc builds and analysis share one toolchain.
- Evaluate whether `misc-include-cleaner` remains worth the churn once toolchains are aligned; disable or scope it if it keeps producing low-value noise.

## API / Design Questions

- Reconsider whether unconditional conversion to `HighPrecisionFloat` is the right default for all algorithms.
- Document the tradeoff explicitly:
  - better numeric stability and one internal representation
  - possible performance cost, wider ABI surface, and different behavior across platforms where `long double` differs
- Evaluate alternatives:
  - keep `HighPrecisionFloat` as the internal default but make it easier to swap or configure
  - use native input/result types in more paths and reserve `HighPrecisionFloat` for selected algorithms
  - expose a policy or traits-based customization point for accumulation/result type

## CI / Quality

- Decide whether CI should also publish generated Doxygen docs (for example as a GitHub Pages site, a CI artifact, or both) now that pull requests already verify doc generation and warnings.
- Consider grouping formatting, tidy, and documentation checks so contributor-facing failures are easier to interpret.
