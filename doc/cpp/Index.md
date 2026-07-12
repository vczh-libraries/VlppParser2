# C++ Parsing Documentation

These documents cover the BuiltIn-Cpp test grammar, VlppParser2's suitability for C++26, raw-source frontend boundaries, and concrete standard-syntax gaps.

For grammar maintenance, begin with [C++ Syntax Implementation Philosophy](Philosophy.md), then read [BuiltIn-Cpp Syntax Design](SyntaxDesign.md). For the parser architecture behind it, begin with the main [VlppParser2 design guide](../Index.md).

## Design

- [C++ Syntax Implementation Philosophy](Philosophy.md) defines the code-indexing purpose and the ordered priorities for historical compatibility, orthogonality, bounded over-acceptance, and avoiding unnecessary ambiguity.
- [BuiltIn-Cpp Syntax Design](SyntaxDesign.md) explains rule-file boundaries, all `@public` grammar APIs, orthogonality, context variants, ambiguity, and the declarator subsystem.
- [C++26 Parsing with VlppParser2](Cpp.md) evaluates the parser architecture, frontend boundary, ambiguity model, limitations, and the indexer's semantic boundary.

## Cross-version work

- [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md) separates missing lexer declarations from features requiring stateful scanning, source translation, or preprocessing across all standard editions.
- [De-ambiguation Improvements](Cases_Improvement.md) defines syntax-only normalization, AST reclassification, ambiguity that must remain, and deliberate practical over-acceptance.

## Phase-7 syntax and AST gaps

- [C++17 and Earlier](Cases_17.md) covers missing C++17-or-earlier syntax, including formerly valid standard features retained for indexing historical code.
- [C++20](Cases_20.md) covers C++20 additions such as modules, constraints, coroutines, and new declaration forms.
- [C++23](Cases_23.md) covers C++23 additions and AST-fidelity gaps such as explicit object parameters and multidimensional subscripts.
- [C++26](Cases_26.md) covers the N5046 additions used by this audit, including reflection, contracts, pack indexing, structured-binding extensions, and expansion statements.

## Reading by task

- To change the BuiltIn-Cpp grammar, read [C++ Syntax Implementation Philosophy](Philosophy.md), [Syntax Design](SyntaxDesign.md), and then the relevant versioned gap file.
- To change raw token handling or preprocessing, read [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md) and the frontend boundary in [C++26 Parsing with VlppParser2](Cpp.md).
- To classify ambiguity, read [De-ambiguation Improvements](Cases_Improvement.md) and the ambiguity sections in both design documents; BuiltIn-Cpp keeps lookup-dependent candidates unresolved.
- To assess standard coverage, read the version files chronologically; each file records features introduced in that edition, while the C++17 file also covers earlier editions.

## Working assumptions

BuiltIn-Cpp targets compiler-verified input for a code indexer and follows the priority order in [C++ Syntax Implementation Philosophy](Philosophy.md). It accepts compatible historical spellings unless a newer-standard interpretation or the avoidance of unnecessary ambiguity takes priority, preserves choices that require name or type lookup, and may deliberately accept harmless invalid spellings when doing so keeps the grammar orthogonal without misinterpreting valid input.

Update these documents when the grammar, AST contract, frontend boundary, or audited standard coverage changes semantically. Do not rewrite them merely because code moved or was mechanically refactored without changing the documented behavior.
