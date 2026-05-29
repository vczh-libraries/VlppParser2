# !!!LEARNING!!!

# Orders

- Generate `.d.ts` schemas from AST JSON visitor output [1]
- Host JSON/XML node list serializers in `vl::glr` [1]

# Refinements

## Generate `.d.ts` schemas from AST JSON visitor output

When adding JSON output for generated ASTs, generate a matching `.d.ts` file from the same AST symbol metadata. Use string-literal unions for enums, `$ast` discriminants for concrete classes, union types for abstract classes, and shared `_Common` interfaces for abstract ancestors with properties so TypeScript consumers match the printed JSON exactly.

## Host JSON/XML node list serializers in `vl::glr`

Reusable AST node-list serializers belong in Parser2 headers rather than downstream consumers. Keep JSON node list serialization in `vl::glr::json::JsonNodeListSerializer`; for XML, use `vl::glr::xml::XmlElementListSerializer`, serialize lists by wrapping elements under an `<Array>` element and printing that element with `XmlPrint`, and deserialize by parsing a root element with `XmlParseElement` then copying all child elements regardless of the root name or attributes.
