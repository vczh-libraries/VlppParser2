# TODO

## Next task

- Try to make large AST not causing stack overflow while disposing.
  - Generate code to collect all nodes in any destructor and mark (to tell all sub nodes they are processed)?

## Big Design Change

### New Instructions

- StackBegin() and StackEnd() manage a separated storage of slots.
- CreateObject(type) pushes a new creating object.
- StackSlot(n) pops a creating object and store it to the n-th slot.
- StackBegin() and StackEnd() do not affect the creating object stack.
- Token(n) and EnumItem(v, n) store a value directly to the n-th slot.
- Field(f, n) assign all values in the n-th slot to a field.
- If the first input in a clause is a rule, StackBegin() + optional(StackSlot(n)) is generated after existing the rule.
- If the first input in a clause is a token, StackBegin() is generated before Token(n).
- ResolveAmbiguity(type) merge all objects in 0-th slot.

### Progressing

`Test\Source\BuiltIn-Cpp\Input\Declarations\Generic_Decls\Forward_Members.txt`
```
template... A::B<_1, X>::C::D<Y, _2>::D(X, Y){}
```
Ambiguous VariableDeclaration but trying to accept a QualifiedName(A)

`Test\Source\BuiltIn-Cpp\Input\Declarations\GenericPS_Decls\ForwardDecl_CtorsDtors.txt`
```
template... A::B<_1, X>::C::D<Y, _2>::D(){}
```
Probably the same reason

`Test\ParserLog\BuiltIn-Cpp\Trace-1[File_AmbiguousDecl4].txt`
Looks good, but in `Trace-3` there is
```
[17]: 6@0 - 6@1
[18]: 7@0 - 7@4
[19]: 8@0 - 9@8
[10]: 10@0 - 205@0
[11]: RA_Branch
[21]: 6@0 - 6@1
[22]: 7@0 - 7@4
[9]: 8@0 - 9@8
[12]: 11@0 - 205@0
[13]: RA_Branch
[7]: 6@0 - 6@1
[8]: 7@0 - 7@4
[14]: RA_Branch
```
The third branch should continue from 14@0 to 204@0

```
0..7
  +----------------------+
  |                      |
8..9                     |
  +----------+           |
  |          |           |
10..193   11..192        |
  +----------+           |
  |                      |
179..205              18..204
  +----------------------+
  |
182..184
```

- [x] Non-ambiguous test cases
- [x] Ambiguous test cases
- [x] Split FeatureTest
- [x] Built-in parsers:
  - [x] Json
  - [x] Xml
  - [x] Workflow
- [x] prefix_merge test cases
  - [x] merge prefix in rules
    - 109236 -> 10141 -> 6663 states: `Test\ParserLog\BuiltIn-Workflow\Trace-1[Codegen_WorkflowHints].txt`, meanwhile 6750 in master
  - [x] automatically identify prefix_merge
- [ ] Built-in parsers: C++
- [ ] build.ps1
- [ ] Reorganize log utilities for better dependency
- [ ] Render ambiguity with not only traces but input codes in Trace-3
- [ ] Document design principal, algorithm and syntax
- [ ] Finish `## Features to Add`
- [ ] build.ps1
- [ ] Build in Ubuntu. Windows and Linux test output inconsistency on
    - the order of ambiguous candidates.
    - `\r\n` or `\n` serialized into `<![CDATA[]]>`.
    - We can force `\r\n` in unit test, normalizing all inputs.
- [ ] Built-in parsers: C++ Non-traced test cases save json files with extra field recording the input code at the beginning

## Prefix Merge

In `PrefixMerge5_Pm` test case, one of a prefix merge instance is
```
  [RULE] _PrimitiveShared :
    _LongType -> _PrimitiveShared
    _Expr -> _Expr1 -> _Expr0 -> _PrimitiveShared
    _Expr -> _Expr1 -> _Expr0 -> _LongType -> _PrimitiveShared
```
Currently we only do `_PrimitiveShared` merging.
In the future we should do
```
_PrimitiveShared +-> _Expr0 ... _Expr
                 +-> _LongType +-> _LongType
                               +-> _Expr0 ... _Expr
```
Or we are getting this
```
[59][Module] BEGIN [pm-cr-rule: _PrimitiveShared]
[RULE: 8]
	ending -> [63][Module] END [ENDING]
		+ StackBegin()
		+ StackEnd()
	ending -> [63][Module] END [ENDING]
		+ StackBegin()
		+ StackEnd()
	leftrec -> [27][_LongType]< _LongType @ "(" { _LongType ; "," } ")" >
		+ StackBegin()
		+ StackEnd()
		+ StackBegin()
		+ StackSlot(0)
		> rule: _LongType -> [65][Module]<! !_LongType @ !>
			+ StackBegin()
	leftrec -> [27][_LongType]< _LongType @ "(" { _LongType ; "," } ")" >
		+ StackBegin()
		+ StackEnd()
		+ StackBegin()
		+ StackSlot(0)
		> rule: _Expr -> [64][Module]<! !_Expr @ !>
			+ StackBegin()
		> rule: _Expr1 -> [56][_Expr]<! !_Expr1 @ !>
			+ StackBegin()
		> rule: _Expr0 -> [50][_Expr1]<! !_Expr0 @ !>
			+ StackBegin()
		> rule: _LongType -> [40][_Expr0]< _LongType @ "{" { _Expr ; "," } "}" >
			+ StackBegin()
			+ StackSlot(0)
```

## Features to Add

- Extensible tokens, for example, recognize `R"[^\s(]\(` and invoke a callback function to determine the end of the string.
  - `RegexTokenizer`
  - New syntax for tokenizers for such extensible tokens.
  - We can try `/***/` with extensible tokens.
- AST file groups.
  - An AST file only sees:
    - [x] Types defined in this file.
    - [x] `@public` types defined in the same file group.
    - [ ] `@extern` types defined in different **depended** file groups **as field type only**.
  - C++ codegen are created per groups.
    - Only AST classes `#include` depended files groups, visitors do not.
    - When a visitor need to call types in different file groups, leave it abstract.

## Test Cases

- Code Coverage
  - Collect uncovered code again by break points in executator (trace manager).
- Reconsider in new implementation:
  - Test `SyntaxSymbolManager::PrefixMergeCrossReference_Solve` firmly.
  - Create ambiguity test case caused by only one clause with alternative syntax.

## Issues (BuiltIn-Cpp)

- `::a::b::c::*`
  - Ambiguity
  - It should be invalid, because `::a::b::c` are always parsed as one QualifiedName, instead of being `::a(::b::c::*)` and `::a::b(::c::*)`
  - Refer to `Priority in left recursive transition`
- Compiler crashes:
  - `_DeclOrExpr ::= !_BExpr ::= {_DeclaratorKeyword:keywords} _TypeBeforeDeclarator:type _DeclaratorRequiredName:declarator as DeclaratorType ;`
  - `workingSwitchValues` is nullptr in `ExpandClauseVisitor::FixRuleName`
- Write a powershell script to generate all `.i` files from `.cpp` and `.h` in every `Source` folder, collect them in a central place to create test cases.
  - `Release\IncludeOnly` will be useful to resolve cross-repo dependencies.

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

- Indirect and multiple left recursion.
- Twist slot number in alternative branches in a clause and see if it is possible to merge prefix
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
