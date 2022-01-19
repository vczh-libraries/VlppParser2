# TODO

## Features to Add

- Add switches and push-pop syntax.
  - switch can only be boolean.
  - switch must have default value.
  - `(OPTION1, !OPTION2, ...; SYNTAX)` assigns `true` to `OPTION1` and `false` to `OPTION2` etc, runs `SYNTAX`, and restores.
  - condition syntax: `?(CONDITION1:SYNTAX1 | ...)`.
    - `SYNTAX1` is valid only if `CONDITION1` is evaluated to `true`.
    - all branches must be conditional.
    - all branches must not consume empty sequence except `;`.
    - if a clause is a left recursive clause, there must be no condition pushing or testing from the start state to the left recursive rule syntax.
    - if all conditions fail, the syntax fail.
    - `CONDITION?;` means, if the condition is evaluated to `true`, this syntax consumes no input.
      - only one branch could be `;`.
  - condition could be:
    - `OPTION`: means `OPTION == true`
    - `C1 && C2`
    - `C1 || C2`
    - `!C`
    - `(C)`
- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.

## Issues

- Fix todo in `TraceManager::AreTwoEndingInputTraceEqual`.
- Print correct codeRange for `ParserErrorType::RuleIsIndirectlyLeftRecursive`.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states).
  - Issue: `X ::= ([a] | [b]) c` fails because when both optional syntax fail it creates two trace routes to c and causes ambiguity.
  - Possible solution: if multiple combinations of consecutive epsilon transitions makes an epsilon transition between two states, treat them as one single epsilon transition.
    - Merge conditions in these epsilon transitions properly.
- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.
- Review all comments.

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
