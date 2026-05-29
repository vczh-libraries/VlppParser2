# !!!LEARNING!!!

# Orders

- Verify generated JSON schemas with a TypeScript type-checking package [2]
- Run Parser2 generator test projects in dependency order [1]

# Refinements

## Verify generated JSON schemas with a TypeScript type-checking package

For generated AST JSON schemas, add a small `Test\TypeScript` package that copies generated `.d.ts` files and creates TypeScript files from real generated JSON outputs. `npm run build` should run `tsc --noEmit` in strict mode so schema drift is caught by type checking, in addition to the normal C++ generator/unit-test sequence.

## Run Parser2 generator test projects in dependency order

For Parser2 public header, parser, printer, or serializer changes, run the generator-style unit projects in the documented order and rebuild after the projects that generate code. Finish with the built-in JSON, XML, Workflow, and C++ tests, then run `Test\TypeScript\prepare.ps1` followed by `npm run build` so generated declarations and real JSON examples are checked together.
