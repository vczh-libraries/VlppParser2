# !!!LEARNING!!!

# Orders

- Verify generated JSON schemas with a TypeScript type-checking package [1]

# Refinements

## Verify generated JSON schemas with a TypeScript type-checking package

For generated AST JSON schemas, add a small `Test\TypeScript` package that copies generated `.d.ts` files and creates TypeScript files from real generated JSON outputs. `npm run build` should run `tsc --noEmit` in strict mode so schema drift is caught by type checking, in addition to the normal C++ generator/unit-test sequence.
