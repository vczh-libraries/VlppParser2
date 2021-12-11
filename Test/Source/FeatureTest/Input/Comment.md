The following inputs are not included

- nested_positive_optional++
- nested_positive_optional+++

because they finished rule `PositiveNO1`,
so that candidates that skipped `[Plus:optional]` lose the competition,
but eventually survived candidates are not able to reach the end of the input,
so they become invalid input.