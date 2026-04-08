# Issue: Rework `sum` return type for integral ranges

## Problem

`sum` currently returns `RangePublicResultType<R>`, so integral inputs produce `double`.
That matches the statistical APIs, but it is surprising for a low-level numeric helper and does not align with the direction established for `minMaxValue`.

## Scope

- decide a natural widened result type for integral sums
- keep floating-point sums in a floating-point type
- preserve the current single-pass accumulation behavior
- do not change `product` or `sumSquared` in this pass

## Open design point

`sum` should likely return a widened arithmetic type for integral inputs rather than the raw range value type, because accumulation can overflow narrower element types much earlier than the final result would suggest.

## Acceptance

- define and document the result-type policy for integral sums
- add tests that pin both the type and the computed value
- confirm the higher-level statistical functions still use their existing public result types
