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
- Check `prefix_merge` errors
  - If a rule indirectly begins with `prefix_merge`:
    - all clauses should begin with `prefix_merge`
      - put `CHECK_FAIL(L"Not Implemented!")` during generating instructions since it must be allowed to allow `throw expression` in expressions.
  - Any rule should not go through switch instructions and reach PM/LRP in one token input.
  - LRI should not inject through switch instructions.
- Add `throw expression` to `PrefixSubset5` to test `prefix_merge`.

## Issues

- Test `X ::= X LRI ...`.
- Optimize `CalculateFirstSet_IndirectStartRules` using partial ordering.
- TODO in `SyntaxSymbolManager::EliminateSingleRulePrefix`.
  - Deny `A ::= !B ::= B as Something ::= ...;`.
- Print correct codeRange for:
  - `ParserErrorType::RuleIsIndirectlyLeftRecursive`
  - `ParserErrorType::LeftRecursiveClauseInsidePushCondition`
  - `ParserErrorType::LeftRecursiveClauseInsideTestCondition`
- `X ::= ([a] | [b]) c` fails because multiple optional syntax create multiple epsilon transition between the same pair of states.
  - Possible solution: if multiple combinations of consecutive epsilon transitions makes an epsilon transition between two states, treat them as one single epsilon transition.
    - Merge conditions in these epsilon transitions properly.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states).
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
