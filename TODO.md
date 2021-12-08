# TODO

## Experiments

- Serializing
  - Escaping and Unescaping pairs (instead of only unescaping)
  - Calculate ambiguous **ToString** cases
  - Generate **ToString** algorithm
- Loop priority
- Document the algorithm in a markdown file

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

## Development

1. [x] `ParserTest_ParserGen_Compiler`
   1. [ ] Prepare more parser test cases for advanced features
      1. [x] `Calculator.
      2. [x] `IfElseAmbiguity`. (Equal)
      3. [x] `IfElsePriority`. (PreferTake)
      4. [x] `GenericAmbiguity` (Ambiguity in loop, ambiguity in left recursive clauses)
      5. [ ] `IfElsePriority2`. (PreferSkip, but don't let `-[C]` ends a clause (add compile error), or it will always win)
      6. [ ] `GenericPriority` (?)
   2. [ ] Generate JSON parser
   3. [ ] Generate XML parser
1. [x] `ParserTest_ParserGen_Generated`
   1. [x] Prepare more parser test cases (including `Calculator/Generated`).
   2. [ ] Run all parsers.
   3. [ ] Test JSON parser.
   4. [ ] Test XML parser.
3. [ ] Port `CodePack` and `ParserGen` to `VlppParser2`, do not write to an existing file if the content is not changed.
   1. [ ] Create a new repo `BuildTools` and adapt the release license instead of the development license.

## Work Items (GLR features)

- [x] Ambiguity
- [x] Priority
- [ ] Priority in alternative syntax, but don't let it ends a clause if a high priority branch could be empty (add compile error)
- [ ] Priority in returnDesc / left recursive transition
- [ ] Priority with left recursion
- [ ] Priority in different clauses of the same rule (`TraceManager::CheckAttendingCompetitionsOnEndingEdge`)
- [x] Ambiguity with left recursion, when ASTs creates from left recursion clauses belong to a bigger part of ambiguity resolving
- [x] Ambiguity with left recursion, when two left recursive clauses consume the same series of inputs
- [ ] Priority with ambiguity
- [ ] Everything together

## Work Items (issues)

- fix todo in `TraceManager::AreReturnDescEqual`
- fix todo in `TraceManager::AreReturnDescEqual`
- fix todo in `TraceManager::CheckAttendingCompetitionsOnEndingEdge`
- `EndObject` after `ReopenObject` doesn't update `ParsingAstBase::codeRange::start`.
  - for example, when `Exp` is reopened to run `( Exp @ )`, then the created ast begins from `Exp` but ends at `)`.
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
- Printing AST classes that created from a memory pool.
- Error message generating.
  - Allow users to customize error messages.
  - Support localization.
- Error recovering.
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.
- Escaping and unescaping functions
  - Offer two options: experiment
  - Map positions between escaped and unescaped text
