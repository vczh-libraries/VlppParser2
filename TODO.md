# TODO

## PrefixSubset3

- Add test cases with constructors mixing with binary expressions.

## Features to Add

- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.
- Add `prefix_merge` clause which will be written to `left_recursion_placeholder`, `left_recursion_inject` and `left_recursion_inject_multiple`.
  - A `prefix_merge` clause can only be used on a single `!RULE` syntax, e.g. `prefix_merge !RULE`.
  - When a clause is compiled, it will searches and see if starting rules are all `prefix_merge` rules
    - If none, leave it.
    - If all, rewrite it using `left_recursion_placeholder`, `left_recursion_inject` and `left_recursion_inject_multiple`.
    - If mixed, report errors.

## Issues

- Deny when `A LRI(Flag) (B ...) | (C ...)` happens, `B` is a prefix of `C`, and both injection could end with a same target X.
- In `PrefixSubset4`, find a way to optimize `left_recursion_inject_multiple(Shared) _Expr | _LongType`
- TODO in `SyntaxSymbolManager::EliminateSingleRulePrefix`.
  - Deny `A ::= !B ::= B as Something ::= ...;`.
- Print correct codeRange for:
  - `ParserErrorType::RuleIsIndirectlyLeftRecursive`
  - `ParserErrorType::LeftRecursiveClauseInsidePushCondition`
  - `ParserErrorType::LeftRecursiveClauseInsideTestCondition`
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

- Eliminate `Switches::values` size limit.
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
- Error if any condition is constantly evaluated to true or false
