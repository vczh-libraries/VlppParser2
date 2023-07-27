# TODO

## BuiltIn-Cpp

- Progress
  - When `lri` could end the current rule, `leftrec` into the deepest place instead of `ending` to the ending state of the current rule.
    - Do not rewrite a rule to LRI if there is only one clause without branches.
    - Revert `FixLeftRecursionInjectEdge generate Token+LeftRec insead of Token+Ending` if no longer needed.

## Next task

- `@ambiguous class X : T { ... }` when X has members:
  - Becomes `@ambiguous class X : T {}` and `class XCommon : X { ... }`.
  - Ast:
    - `class Y : X` -> `class Y : XCommon`.
    - `var value : X` or `var value : X[]` unchanged.
  - Syntax
    - `as X` -> `as XCommon`.
    - `as partial X` -> `as partial XCommon`.
    - If a rule's type deducted to `XCommon`, change it to `X`.
- Add `extern` rule, non-`extern` rules can only be used inside the same syntax file.
- Add `extern` type, non-`extern` types can only be used inside the same ast file.
  - AST uses classes from another AST file in dependency as fields.
- Rewrite and remove switch before removing PrefixMerge.
  - Rename `LeftRecursionPlaceholderMixedWithSwitches`
- Multiple LRI following one Target
- Generate multiple level of LRI from prefix_merge
  - Remove `PrefixExtractionAffectedRuleReferencedAnother`
  - Currently it generates an error if 3 levels are required
  - Allow one prefix followed by multiple continuations
    - Optional applies to all continuations as a whole

## Test Cases

- `PrefixMerge` 1-7 add test cases to parse expr and type separately.
- Create test cases that only rewrite syntax without generating C++ code for:
  - `DeductEmptySyntaxVisitor`
  - Invalid combined clauses during expanding switches
  - everything else that is needed
- TODO(s) in `RewriteRules_GenerateAffectedLRIClausesSubgroup`.
- TODO(s) in `CalculateObjectFirstInstruction` and `InjectFirstInstruction`.
- Make a test case to test `prefix_merge` generates `left_recursion_inject_multiple`.
- Create ambiguity test case caused by only one clause with alternative syntax.
- Test when an object get LriFetch to multiple branches following a ReopenObject.
- Deny `X ::= Y LRI ...` when `X` is or a prefix of `Y`.
- Windows and Linux test output inconsistency on
  - the order of ambiguous candidates.
  - `\r\n` or `\n` serialized into `<![CDATA[]]>`.

## Features to Add

- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - Offer two options: using (rich regex | C++) to search for complete token.
- Add union type and remove `TypeOrExprOrOthers` in C++.

## Issues (BuiltIn-Cpp)

- `::a::b::c::*`
  - Ambiguity
  - It should be invalid, instead of being `::a(::b::c::*)` and `::a::b(::c::*)`
- Compiler crashes:
  - `_DeclOrExpr ::= !_BExpr ::= {_DeclaratorKeyword:keywords} _TypeBeforeDeclarator:type _DeclaratorRequiredName:declarator as DeclaratorType ;`
  - `workingSwitchValues` is nullptr in `ExpandClauseVisitor::FixRuleName`

## Issues (Glr)

- When `XToResolve` is in another `XToResolve`, flatten them.
- TODO in `CalculateRuleAndClauseTypes`.
- TODO in `ValidateDirectPrefixMergeRuleVisitor`.
- Optimize `CalculateFirstSet_IndirectStartRules` using partial ordering.
- TODO in `SyntaxSymbolManager::EliminateSingleRulePrefix`.
  - Deny `A ::= !B ::= B as Something ::= ...;`.
- TODO in `CalculateObjectLastInstruction`
- TODO in `CheckAmbiguityResolution`
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

- Try to see if it is possible to
  - Remove `PushReturnStack` last argument.
  - Remove `ReturnDesc::ruleType`.
  - Move `ReturnRuleType` from automaton to symbol.
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
