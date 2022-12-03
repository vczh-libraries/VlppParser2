# TODO

## Progressing

- `[_GenericArgument]<< !_PrimitiveType @ ( lri:_GenericArgument_LRI_Original ) >>` should accept `ending`
  - because `_TypeOrExpr_NoComma_NoGT:argument ["...":variadic] as GenericArgument` could jumps to `@:argument` by `int`
  - could be related to `_QualifiedName` does not have an optional LrInject clause
- `throw` is missing in `BuiltIn-Cpp` in `_TypeOrExpr`
- `_GenericArguments` generates `token: "operator" -> [71][_OperatorIdentifier]< "operator" @ "!" >` twice
  - could be related to `_QualifiedName` does not have an optional LrInject clause

## Next task

- Rewrite and remove switch before removing PrefixMerge.
  - Rename `LeftRecursionPlaceholderMixedWithSwitches`
- Multiple LRI following one Target
- Generate multiple level of LRI from prefix_merge
  - Remove `PrefixExtractionAffectedRuleReferencedAnother`
  - Currently it generates an error if 3 levels are required

## Test Cases

- `PrefixMerge` 1-7 add test cases to parse expr and type separately.
- Create test cases that only rewrite syntax without generating C++ code for:
  - `DeductEmptySyntaxVisitor`
  - Invalid combined clauses during expanding switches
  - everything else that is needed
- TODO(s) in `RewriteRules_GenerateAffectedLRIClausesSubgroup`.
- Make a test case to test `prefix_merge` generates `left_recursion_inject_multiple`.
- Create ambiguity test case caused by only one clause with alternative syntax.
- Test when an object get LriFetch to multiple branches following a ReopenObject.
- Deny `X ::= Y LRI ...` when `X` is or a prefix of `Y`.

## Features to Add

- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.

## Issues

- When `XToResolve` is in another `XToResolve`, flatten them.
- TODO in `CalculateRuleAndClauseTypes`.
- TODO in `ValidateStructurePrefixMergeRuleVisitor`.
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

## BuiltIn-Cpp

- Progress: Only prepared test cases from `Input_Sample/(Exprs|Types).txt`, ignoring lambda expressions.

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
- Error if any condition is constantly evaluated to true or false
