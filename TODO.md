# TODO

## Experiments

- Full CFG power, no limitation
  - Experiment: expanding all left-recursive grammer to right-recursive grammar with instructions
  - Experiment: optionally inline all rules which don't generate parser functions
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
  - Multiple syntax files are combined together before creating `SyntaxSymbolManager`.
- [x] All names of AST must be unique globally.
- [x] All names of token and rule must be unique.
- [x] Unique field id will be generated for each fields in each AST node.
- [ ] One XML "parser project file" to contain all above information, linking to all AST/Lexical Analyzer/Syntax files.

## EBNF Program

- Priority of loops:
  - `+[ RULE ]` (RULE is optional): if `RULE` succeeds, another failure branch is not considered even if the rest doesn't parse.
  - `-[ RULE ]` (RULE is optional): skip `RULE` and parse the rest. If failed, recognize `RULE` and redo **only the current named-clause**.
  - `[ RULE ]` (RULE is optional): keep both results
  - `+ { RULE }` (RULE is repetitive): recognize as much `RULE` as possible first, and then do the rest.
  - `- { RULE }` (RULE is repetitive): recognize as less `RULE` as possible, and the do the rest. If failed, recognize one more `RULE` and redo **only the current named-clause**.
  - `{ RULE }` (RULE is repetitive): keep all results.
  - The loop body could be `RULE; DELIMITER` to specify delimiters between loop items. A delimiter could also be a rule.

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

1. [x] `ParserTest_AstGen`.
   1. [x] Symbol table for AST.
   2. [x] Manually create a symbol table for the AST for `ParserGen`.
   3. [x] Symbol table -> C++ code.
   4. [x] Generate `ParserGen` AST C++ files in this unit test project.
      1. [x] AST for `ParserGen::TypeAst`.
      2. [x] AST for `ParserGen::RuleAst`.
      3. [x] Ast for `Calculator`
      4. [x] Visitors
      5. [x] Assembler
2. [x] `ParserTest_AstParserGen`.
   1. [x] Define instructions.
   2. [x] Test instructions.
   3. [ ] Test copy visitors.
   4. [ ] Test traverse visitors.
   5. [x] Test json visitors.
   6. [x] Generate `ParserGen` Lexer C++ files in this unit test project.
      1. [x] Lexer for `ParserGen`.
      2. [x] Lexer for `Calculator`.
3. [x] `ParserTest_LexerAndParser`.
   1. [x] Test generated lexer for `Calculator`.
   2. [x] Manually create parser symbols for `Calculator`.
      1. [x] `SyntaxSymbolManager` accepts epsilon-NFA state machines.
      2. [x] Syntax writer for `SyntaxSymbolManager`, it allows us to write state machines like EBNF, for testing purpose only.
      3. [x] Create `Calculator` state machines using the writer.
      4. [x] Process the state machines and generate parser-ready data structures from `SyntaxSymbolManager` with AST structure data input.
   3. [x] Test the parser.
      1. [x] Test and assert `export 1`.
      2. [x] Prepare input text files in `Test/Source/Calculator/Input/`.
      3. [x] Prepare output JSON files in `Test/Source/Calculator/Output/`.
      4. [x] Run the generated `Executable` from `SyntaxSymbolManager`.
      5. [ ] Check `codeRange` of all nodes. (use the previous/current token(calculate codeRange) v.s. before/after rule deduction (move instructions to different places))
   4. [x] Generate `ParserGen` Syntax C++ files in this unit test project.
      1. [x] Syntax for `ParserGen::TypeSyntax`.
      2. [x] Syntax for `ParserGen::RuleSyntax`.
      3. [x] Syntax for `Calculator`.
4. [x] `ParserTest_ParserGen_Compiler`
   1. [x] Initialize `AstSymbolManager` from `ParserGen::TypeAst`.
   2. [x] Initialize `SyntaxSymbolManager` from `ParserGen::SyntaxAst`.
   3. [x] Test generated `Calculator` syntax C++ files again.
   4. [x] Test generated `ParserGen` syntax C++ files to parse `Test/Source/Calculator/Syntax/Ast.txt`.
   5. [x] Test generated `ParserGen` syntax C++ files to parse `Test/Source/Calculator/Syntax/Syntax.txt`.
   6. [x] Generate the whole `Calculator` C++ files in `Test/Source/Calculator/Parser2` again.
   7. [ ] Prepare more parser test cases for `+loop`, `-loop`, `loop`, `+opt`, `-opt`, `opt`, left recursion, indirect left recursion, ...
      1. [ ] Multiple parser syntax under `Test/Source/*/Syntax/`.
      2. [ ] Generate C++ files from them in `Test/Source/*/Parser/`.
      3. [ ] Prepare input text files in `Test/Source/*/Input/`.
      4. [ ] Prepare output JSON files in `Test/Source/*/Output/`.
      5. [ ] Use the generated `ParserGen` syntax C++ files to do above work items.
   8. [ ] Prepare more parser test cases for compile errors, no need to have input/output/codegen
   9. [ ] Generate JSON parser
   10. [ ] Generate XML parser
5. [ ] `ParserTest_ParserGen_Generated`
   1. [ ] Prepare more parser test cases (including `Calculator/Parser2`).
      1. [ ] Prepare input text files in `Test/Source/*/Input/`.
      2. [ ] Prepare output JSON files in `Test/Source/*/Output/`.
   2. [ ] Run all parsers.
   3. [ ] Test JSON parser.
   4. [ ] Test XML parser.
6. [ ] Port `CodePack` and `ParserGen` to `VlppParser2`, do not write to an existing file if the content is not changed.
   1. [ ] Create a new repo `BuildTools` and adapt the release license instead of the development license.

## Work Items (before `ParserTest_ParserGen_Compiler`)

- `EndObject` after `ReopenObject` doesn't update `ParsingAstBase::codeRange::start`.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states)

## Work Items

- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.
- Switching lexical analyzer during parsing.
  - Refactor some properties in `LexerSymbolManager` into `LexerFile` with a name.
- AST uses classes from another AST file in dependency as fields.
- Ambiguity AST and parsing.
- Printing AST classes that created from a memory pool.
- Support multiple syntax definition file in one parser.
- Error message generating.
  - Allow users to customize error messages.
  - Support localization.
- Error recovering.
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
