# TODO

## Experiments

- Serializing
  - Escaping and Unescaping pairs (instead of only unescaping)
  - Calculate ambiguous **ToString** cases
  - Generate **ToString** algorithm

## Configurations

- [x] Include files in generated C++ header.
  - [ ] Default to `<VlppParser2.h>`
- [x] Standalone AST definition file, instead of being part of the syntax like `VlppParser`.
  - [ ] Global UTF configuration for all `token` in a parser.
  - [ ] An AST node in one definition files cannot inherits from one in another file, it could only use one in another file as fields.
  - [x] Reflection supports (opt-out using `VCZH_DEBUG_NO_REFLECTION`).
    - [ ] If AST memory pool is enabled, add the pool object to the constructor in reflection, redirecting to a static function.
    - [ ] All `token` property `X` becomes `X_`, paired with a string property `X` to access the text value in `X_`.
- [x] Standalone lexical analyzer definition files.
- [x] Standalone syntax analyzer definition files.
  - [ ] One parser creates one `SyntaxSymbolManager`, could include multiple syntax files.
- [x] All names of AST must be unique globally.
- [x] All names of token and rule must be unique.
- [x] Unique field id will be generated for each fields in each AST node.
- [ ] One XML "parser project file" to contain all above information, linking to all AST/Lexical Analyzer/Syntax files.

## EBNF Program

- RULE {`::=` CLAUSE [`as` CLASS-NAME [`{` {ASSIGNMENT ...} `}`]] } `;`
  - Consider a syntax here to switch to different lexical analyzer.
  - The type of a rule is not specified, it can be inferred to:
    - `token`: if all clauses are single-token syntax.
    - `partial` TYPE: if all clauses have the same type `SYNTAX as partial TYPE {...}`. All `TYPE` must be exactly the same.
      - A partial type rule could be used in another rule if its type is:
        - `partial` TYPE2, while TYPE2 should be the same to TYPE or its derived type
        - TYPE2, while TYPE2 should be the same to TYPE or its derived type
    - TYPE: if all clauses are `SYNTAX as TYPE {...}` or `SYNTAX containing one !RULE`.
      - the type of this rule is the most detailed common base types of all `TYPE`.

## Development

1. [x] `ParserTest_ParserGen_Compiler`
   1. [ ] Prepare more parser test cases for advanced features (proprities) ...
      1. [ ] Multiple parser syntax under `Test/Source/*/Syntax/`.
      2. [ ] Generate C++ files from them in `Test/Source/*/Parser/`.
      3. [ ] Prepare input text files in `Test/Source/*/Input/`.
      4. [ ] Prepare output JSON files in `Test/Source/*/Output/`.
      5. [ ] Use the generated `ParserGen` syntax C++ files to do above work items.
   2. [ ] Generate JSON parser
   3. [ ] Generate XML parser
1. [x] `ParserTest_ParserGen_Generated`
   1. [x] Prepare more parser test cases (including `Calculator/Generated`).
   2. [ ] Run all parsers.
   3. [ ] Test JSON parser.
   4. [ ] Test XML parser.
3. [ ] Port `CodePack` and `ParserGen` to `VlppParser2`, do not write to an existing file if the content is not changed.
   1. [ ] Create a new repo `BuildTools` and adapt the release license instead of the development license.

## Work Items (issues)

- `EndObject` after `ReopenObject` doesn't update `ParsingAstBase::codeRange::start`.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states).
- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.
- `ParserTest_AstParserGen`
  - Test Copy Visitors
  - Test Traverse Visitors
- `ParserTest_LexerAndParser`
  - Check `codeRange` of all nodes. (use the previous/current token(calculate codeRange) v.s. before/after rule deduction (move instructions to different places))

## Work Items (enhancement)

- Switching lexical analyzer during parsing.
  - Refactor some properties in `LexerSymbolManager` into `LexerFile` with a name.
- AST uses classes from another AST file in dependency as fields.
- Branch priorities:
  - Loop
  - Optional
  - Alternative
- Printing AST classes that created from a memory pool.
- Support multiple syntax definition file in one parser.
- Error message generating.
  - Allow users to customize error messages.
  - Support localization.
- Error recovering.
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.
