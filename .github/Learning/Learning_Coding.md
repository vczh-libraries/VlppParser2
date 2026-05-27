# !!!LEARNING!!!

# Orders

- Generate `.d.ts` schemas from AST JSON visitor output [1]

# Refinements

## Generate `.d.ts` schemas from AST JSON visitor output

When adding JSON output for generated ASTs, generate a matching `.d.ts` file from the same AST symbol metadata. Use string-literal unions for enums, `$ast` discriminants for concrete classes, union types for abstract classes, and shared `_Common` interfaces for abstract ancestors with properties so TypeScript consumers match the printed JSON exactly.
