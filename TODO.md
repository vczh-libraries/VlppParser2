# TODO

## Experiments

- Full CFG power, no limitation
  - Experiment: expanding all left-recursive grammer to right-recursive grammar with instructions
  - Experiment: optionally inline all rules which don't generate parser functions
- Serializing
  - Escaping and Unescaping pairs (instead of only unescaping)
  - Calculate ambiguous **ToString** cases
  - Generate **ToString** algorithm

## AST Definition

- Must be compatible with Workflow (if possible)

## Configurations

- [x] Include files in generated C++ header.
  - [ ] Default to `<VlppParser2.h>`
- [x] Standalone AST definition file, instead of being part of the syntax like `VlppParser`.
  - [ ] Global UTF configuration for all `token` in a parser.
  - [ ] An AST node in one definition files cannot inherits from one in another file, it could only use one in another file as fields.
  - [x] Reflection supports (opt-out using `VCZH_DEBUG_NO_REFLECTION`).
    - [ ] If AST memory pool is enabled, add the pool object to the constructor in reflection, redirecting to a static function.
    - [ ] All `token` property `X` becomes `X_`, paired with a string property `X` to access the text value in `X_`.
- [ ] Standalone lexical analyzer definition files, with a name.
- [ ] Standalone syntax analyzer definition files.
- [x] All names of AST must be unique globally.
- [ ] All names of token and rule must be unique.
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

- RULE [: TYPE] {`::=` CLAUSE [`as` CLASS-NAME [`with` `{` {ASSIGNMENT ...} `}`]] } `;`
  - Consider a syntax here to switch to different lexical analyzer.
  - The type of a rule could be:
    - `token`
    - `partial` TYPE. Another rule that calls this rule must be
      - either a `partial` TYPE2, while TYPE2 should be the same to TYPE or its derived type
      - TYPE2, while TYPE2 should be the same to TYPE or its derived type
    - TYPE
    - no type, using the result from one fragment in CLAUSE (exp ::= '(' !exp ')')

## Development

1. [x] AST.
   1. [x] Symbol table for AST.
   2. [x] Manually create a symbol table for the AST for `ParserGen`.
   3. [x] Symbol table -> C++ code.
   4. [ ] Generate `ParserGen` AST C++ file in this unit test project.
      1. [x] AST for AST.
      2. [ ] AST for lexicaly analyser.
      3. [ ] AST for syntax.
2. [ ] Instructions.
   1. [ ] Define and test instructions.
   2. [ ] AST + Instruction -> C++ SAX-like callback interface.
   3. [ ] AST + Instruction -> C++ Default implementation for the interface.
   4. [ ] Generate `ParserGen` AST creation C++ file in this unit test project.
3. [ ] Parser.
   1. [ ] Manually create multiple parsers using the `ParserGen` AST, with test input / output.
   2. [ ] Implement the compiler that accepts AST for AST / lexical analyzer / syntax, and create a parser.
   3. [ ] Assert created instructions from parsers.
   4. [ ] Binary serialization and deserialization for parser (lexical analyzer are converted to binary instead of storing regular expressions).
   5. [ ] Generate `ParserGen` parser C++ file in this unit test project.
4. [ ] Testing `ParserGen` parser.
   1. [ ] Recreate above test cases in text format.
   2. [ ] generate C++ code for all of them.
5. [ ] Testing generated parsers.
6. [ ] Create JSON and XML parser.
7. [ ] Port `CodePack` and `ParserGen` to `VlppParser2`, do not write to an existing file if the content is not changed.

## Work Items

- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.
- Switching lexical analyzer during parsing.
- AST uses classes from another AST file in dependency as fields.
- Ambiguity AST and parsing.
- Printing AST classes that created from a memory pool.
- Error message generating.
  - Allow users to customize error messages.
  - Support localization.
- Error recovering.
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
