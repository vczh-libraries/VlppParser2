# TODO

## Roadmap

- Reimplement C++ parser in vczh-libraries/Document using this project.
  - Move all test cases to `BuiltIn_CppDoc`.
- Refactor vczh-libraries/Document to generate document using the new parser but skip the code index temporary.
- Create a new repo `BuildTools` and adapt the release license instead of the development license.
- Review all comments.

## Features to Add

- Add `'LITERAL'` syntax. Unlike `"LITERAL"`, `'LITERAL'` **CANNOT** be any token's display text.
  - For reversed keywords like `abstract`, which could also be a valid identical.
  - in `'LITERAL'`, `''` is an escaping of `'`.
- `'LITERAL'`, `"LITERAL"` and `RULE/TOKEN` can all have field assignments, instead of just `RULE/TOKEN` as today.
- Add switches and push-pop syntax.
  - switch can only be boolean.
  - switch must have default value.
  - `(OPTION1=VALUE1, ...; SYNTAX)` assign `VALUE` to switch `OPTION1` ans so on, run `SYNTAX`, and restores.
  - condition syntax can only be used directly in:
    - clauses: `CONDITION? SYNTAX`
    - alternative branches `(CONDITION? SYNTAX1 | CONDITION? SYNTAX2 | ...)`
      - at most one condition could be just `CONDITION?`, saying this branch accepts nothing when the condition is `true`.
      - it implements a "conditional optional" syntax, only when a condition is `true` it becomes optional.
    - optional `[CONDITION? SYNTAX]`
  - condition could be:
    - `OPTION=VALUE`
    - `A && B`
    - `A || B`
- Add conditional enum field assignment.
  - `{FIELD ?= ITEM}` only assign `ITEM` to `FIELD` if it has not been assigned.
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.

## Issues

- Fix todo in `TraceManager::AreTwoEndingInputTraceEqual`.
- Print correct codeRange for `ParserErrorType::RuleIsIndirectlyLeftRecursive`.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states).
  - Issue: `X ::= ([a] | [b]) c` fails because when both optional syntax fail it creates two trace routes to c and causes ambiguity.
- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.

## Experiments

- Serializing
  - Escaping and Unescaping pairs (instead of only unescaping)
  - Calculate ambiguous **ToString** cases
  - Generate **ToString** algorithm
- Loop priority
- Document the algorithm in a markdown file
- From a given state and a few tokens, the trace graph could be copied directly if:
  - none of state.returnStacks is reduced
  - competitions created before the first token are not attended
  - completitions created after the first token are closed
- Generate LL parser if possible (print error if failed but forced to do)
- Generate SLR parser if possible (print error if failed but forced to do)

## Work Items (enhancement)

- Switching lexical analyzer during parsing.
  - Refactor some properties in `LexerSymbolManager` into `LexerFile` with a name.
- AST uses classes from another AST file in dependency as fields.
- Printing AST classes that created from a memory pool.
- All `token` property `X` becomes `X_`, paired with a string property `X` to access the text value in `X_`.
- New priority syntax
  - Priority in alternative syntax, but all branches must not consume empty input series (add compile error)
  - Priority in left recursive transition (which clause starts this competition?)
- Custom error in syntax.
- Error recovering.
- Escaping and unescaping functions
  - Offer two options: experiment
  - Map positions between escaped and unescaped text
