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

1. [x] `ParserTest_ParserGen_Generated`
   1. [ ] Test JSON parser.
      1. [ ] Escaping / unescaping
      2. [ ] Manual test cases.
      3. [x] Parser output test cases.
   2. [ ] Test XML parser.
      1. [ ] Escaping / unescaping
      2. [ ] Manual test cases.
      3. [x] GacUI DarkSkin test cases.
   3. [ ] Test Workflow parser.
      1. [x] Workflow test cases
      2. [ ] Optimize field assignment / partial rule before use rule, move `{Attribute:attributes}` back in `_Declaration`
2. [ ] Port `CodePack` and `ParserGen` to `VlppParser2`, do not write to an existing file if the content is not changed.
   1. [ ] Create a new repo `BuildTools` and adapt the release license instead of the development license.

## Work Items (issues)

- Fix todo in `TraceManager::AreTwoEndingInputTraceEqual`.
- `EndObject` after `ReopenObject` doesn't update `ParsingAstBase::codeRange::start`.
  - for example, when `Exp` is reopened to run `( Exp @ )`, then the created ast begins from `Exp` but ends at `)`.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states).
  - Issue: `X ::= ([a] | [b]) c` fails because when both optional syntax fail it creates two trace routes to c and causes ambiguity.
- Allow field assignment / partial rule before use rule.
  - Verify ambiguity.
    - `<common prefix> !branch1` with `<common prefix> !branch2`.
    - ambiguity in `<common prefix>`
  - Verify left recursion.
    - `<common prefix> !rule-with-left-recursive`, we cannot assign fields at the beginning of `rule-with-left-recursive`, we must wait until it reduces.
  - Optimize `Delay, Delay, Delay ... End, Reopen, End, Reopen, End, Reopen`
    - Compress multiple consecutive `Delay 1` to `Delay count`
    - Compress multiple consecutive `End, Reopen` to `EndObjectReopenObject count`
- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.
- `ParserTest_AstParserGen`
  - Test Copy Visitors
  - Test Traverse Visitors
- `ParserTest_LexerAndParser`
  - Check `codeRange` of all nodes. (use the previous/current token(calculate codeRange) v.s. before/after rule deduction (move instructions to different places))
- Generate LL parser if possible (print error if failed but forced to do)
- Generate SLR parser if possible (print error if failed but forced to do)

## Work Items (enhancement)

- Switching lexical analyzer during parsing.
  - Refactor some properties in `LexerSymbolManager` into `LexerFile` with a name.
- AST uses classes from another AST file in dependency as fields.
- Printing AST classes that created from a memory pool.
- New priority syntax
  - Priority in alternative syntax, but all branches must not consume empty input series (add compile error)
  - Priority in left recursive transition (which clause starts this competition?)
- Error message generating.
  - Token error messages.
  - Allow users to customize error messages.
  - Support localization.
- Error recovering.
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.
- Escaping and unescaping functions
  - Offer two options: experiment
  - Map positions between escaped and unescaped text
