# Issue: Make `minMaxValue` return natural value types

## Problem

`minMaxValue` currently returns `std::pair<RangePublicResultType<R>, RangePublicResultType<R>>`.
For integral inputs that promotes the result to `double`, which is surprising for a low-level helper that simply reports existing extrema.

## Scope

- change `minMaxValue` to return a widened natural value type instead of the statistical public result type
- keep the statistical APIs and their public result types unchanged
- preserve the current empty-range behavior

## Expected change

- `minMaxValue(std::array<int, N>)` should return `std::pair<int, int>`
- floating-point inputs should keep their existing value type
- mixed or transformed ranges should resolve through the range value type

## Acceptance

- add tests that pin the return type for integral and floating-point inputs
- add tests that verify the runtime min/max values still match the input data
- keep the rest of the library behavior unchanged in this pass
