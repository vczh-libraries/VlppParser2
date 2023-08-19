# TODO

## Next task

- `ExprModule` and `TypeModule` don't generate LRI clauses in `Pm5-8`.

## Test Cases

- Code Coverage
  - Compiler
    - TODO(s) in `RewriteRules_GenerateAffectedLRIClausesSubgroup`.
    - `FixPrefixMergeClauses` in `if (ruleSymbol->isPartial)`.
  - Runtime
    - TODO(s) in `CalculateObjectFirstInstruction` and `InjectFirstInstruction`.
    - `TraceManager::TryMergeSurvivingTraces` in `// if trace is a merge trace`.
    - `TraceManager::BuildStepTree` in which are not covered.
    - `TraceManager::AddTraceToCollection` in `else if (collection == &Trace::predecessors)`
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
- AST file groups.
  - An AST file only sees:
    - Types defined in this file.
    - `@public` types defined in the same file group.
    - `@extern` types defined in different **depended** file groups **as field type only**.
  - C++ codegen are created per groups.
    - Only AST classes `#include` depended files groups, visitors do not.
    - When a visitor need to call types in different file groups, leave it abstract.
- Multiple LRI following one Target
- Generate multiple level of LRI from prefix_merge
  - Remove `PrefixExtractionAffectedRuleReferencedAnother`
  - Currently it generates an error if 3 levels are required
  - Allow one prefix followed by multiple continuations
    - Optional applies to all continuations as a whole

## Issues (BuiltIn-Cpp)

- `::a::b::c::*`
  - Ambiguity
  - It should be invalid, because `::a::b::c` are always parsed as one QualifiedName, instead of being `::a(::b::c::*)` and `::a::b(::c::*)`
  - Refer to `Priority in left recursive transition`
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
  - Or one more phase before generationg automaton for optimization to merge states and edges.
- Optimize `CrossReferencedNFA` to merge prefix (two states can be merged if their `InEdges` are identical, `FromState` in `InEdges` are replaced by merged states).
- `JsonEscapeString` `JsonUnescapeString` handle surrogate pairs correctly.
- Review all comments.

## Experiments

- Add union type and remove `TypeOrExprOrOthers` in C++.
  - Consider what does `@ambiguous union` mean.
- Try to see if it is possible to
  - Remove `PushReturnStack` last argument.
  - Remove `ReturnDesc::ruleType`.
  - Move `ReturnRuleType` from automaton to symbol.
  - Share traces in different branches.
    - From a given state and a few tokens, the trace graph could be copied directly if:
      - none of state.returnStacks is reduced
      - competitions created before the first token are not attended
      - completitions created after the first token are closed
    - Do not copy, share it.
- Serializing
  - Escaping and Unescaping pairs (instead of only unescaping)
  - Calculate ambiguous **ToString** cases
  - Generate **ToString** algorithm
- Generate LL parser if possible (print error if failed but forced to do)
- Generate SLR parser if possible (print error if failed but forced to do)
- Document the algorithm in a markdown file

## Work Items (enhancement)

- Switching lexical analyzer during parsing.
  - Refactor some properties in `LexerSymbolManager` into `LexerFile` with a name.
- Printing AST classes that created from a memory pool.
- All `token` property `X` becomes `X_`, paired with a string property `X` to access the text value in `X_`.
- New priority syntax
  - Priority in alternative syntax, but all branches must not consume empty input series (add compile error)
  - Priority in left recursive transition (which clause starts this competition?)
  - Priority in loop
- Custom error in syntax.
- Error recovering.
- Escaping and unescaping functions
  - Offer two options: experiment
  - Map positions between escaped and unescaped text
- Error if any condition is constantly evaluated to true or false
