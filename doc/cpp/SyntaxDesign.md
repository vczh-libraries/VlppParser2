# BuiltIn-Cpp Syntax Design

This document is the maintenance guide for the C++ grammar in [`Test/Source/BuiltIn-Cpp`](../../Test/Source/BuiltIn-Cpp). It explains the syntax that is implemented now, how the syntax files cooperate, and which rules are cross-file contracts. Proposed additions and coverage gaps belong in the case documents, not in this description of the current grammar.

Read [C++ Syntax Implementation Philosophy](Philosophy.md) before changing this grammar. BuiltIn-Cpp is a code-indexing test parser and a design specimen: it intentionally preserves interpretations that require lookup, accepts bounded compiler-invalid syntax when that keeps the grammar orthogonal, and avoids symbol resolution. Its current standard-coverage gaps are indexed separately in [C++ Parsing Documentation](Index.md).

## Source layout and composition

The parser input has three layers:

- [`Syntax/Lexer.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Lexer.txt) defines the regular-expression lexer used by the test project.
- [`Syntax/Ast`](../../Test/Source/BuiltIn-Cpp/Syntax/Ast) defines the typed AST, including ambiguity-enabled common bases.
- [`Syntax/Syntax`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax) defines grammar rules that assemble those AST types.

[`GenerateCpp.cpp`](../../Test/UnitTest/BuiltInTest_Compiler/GenerateCpp.cpp) lists the AST and syntax files supplied to the parser compiler. Generated files under [`Generated`](../../Test/Source/BuiltIn-Cpp/Generated) are outputs, not the source of the design.

Each syntax file is a box of related definitions. A rule without `@public` is private to that file. A rule marked `@public` may be referenced by rules in another syntax file. This is enforced by the parser compiler, not merely a naming convention.

| Annotation | Contract |
| --- | --- |
| `@public` | Exposes a grammar rule to other syntax files. It is the rule-file API boundary. |
| `@parser` | Generates a C++ parser entry method for the rule. It does not make the rule callable from another syntax file. |
| `@public @parser` | Provides both contracts. `_Type`, `_Expr`, and `_TypeOrExpr` use this combination. |

`_Stat` and `_File` demonstrate the distinction: both are `@parser` entry rules, but neither is a cross-file public rule. Conversely, most `@public` rules exist only to compose the grammar and do not generate C++ entry methods.

All rule symbols are registered before rule bodies are resolved. Therefore the file list is not a topological dependency order: public rules may form deliberate cycles across files. Keep a helper private unless another responsibility box genuinely needs it. Once a rule is public, treat its accepted category and AST result as a maintenance contract.

For the grammar notation itself, including reuse clauses, partial rules, preference operators, and switches, see the [syntax definition reference](../../.github/KnowledgeBase/manual/vlppparser2/syntax.md).

## Governing design principles

[C++ Syntax Implementation Philosophy](Philosophy.md) is the authority for design choices. Its order is significant: accepting compatible history is the baseline; orthogonality may override it; bounded incorrect syntax may be accepted to preserve orthogonality; and avoiding unnecessary ambiguity has the highest priority. The sections below describe how the current grammar realizes that hierarchy.

### Preserve compatible history, but prefer the newer standard

The grammar does not select one input language version. Compatible spellings from different editions share the same rules: for example, `register` remains a declarator keyword and dynamic exception specifications remain accepted beside `noexcept`.

Historical compatibility stops when it would add an avoidable competing interpretation to ordinary modern code. The current grammar therefore gives `auto` its dedicated `QualifiedNameKinds::Auto` form and does not admit it through `_NameIdentifier`. This intentionally rejects its pre-C++11 identifier use instead of making a declaration such as `auto value;` ambiguous for no semantic reason.

### Preserve orthogonality

Every C++ syntactic category should have one canonical implementation. If every place needs a type, all such places should eventually reach `_Type`, `_TypeBeforeDeclarator`, or another explicitly documented type view. Do not copy the productions for a type into template arguments, casts, parameters, base classes, and declarations independently.

The same rule applies to names, expressions, declarators, initializers, statements, and declarations. Extend the rule that owns a concept, then let every consumer receive the extension through its existing dependency.

Some contexts intentionally exclude part of a category. That does not violate orthogonality when the restricted rule is a configured view of the same implementation:

- `_Expr` and `_Expr_NoComma` share one precedence ladder; the latter excludes only the comma operator so a surrounding comma can remain a list delimiter.
- `_TypeOrExpr_NoComma_NoGT` shares the ordinary type-or-expression rules but disables the unparenthesized `>` operator while parsing a template argument.
- Declarator configurations share the same components while requiring, permitting, or forbidding a name.
- `_VarCtorInit` is the parenthesized/braced subset of `_VarInit` for contexts where `=` initialization is not allowed.
- `_DeclNeedSemicolon` and `_DeclRejectSemicolon` route the same declaration families according to who owns the terminator.

A restricted view should express one contextual switch. If it begins accumulating an independent copy of the underlying grammar, the design has stopped being orthogonal.

### Prefer bounded over-acceptance to duplicated grammar

BuiltIn-Cpp parses input that has already been accepted by a compiler. It may therefore accept a small, deliberate superset of standard syntax when doing so produces a much more orthogonal grammar.

This is a design tool, not permission to make every rule broad. Practical over-acceptance is appropriate only when all of the following hold:

- The extra spellings form a small and explainable set.
- No valid spelling loses a candidate or receives a different useful interpretation.
- Tokens and source ranges remain available, so downstream tooling can inspect the spelling without requiring the parser to enforce strict compiler validity.
- The broader rule removes substantial duplication or keeps one syntactic category behind one public API.
- The choice is documented beside the affected coverage and does not conceal a genuinely missing standard construct.

Examples include accepting split operator spellings such as `a > > b` through the same shift rule, admitting declarator keywords in some compiler-invalid orders through the shared declarator combinators, and using one untyped function-shaped declarator for constructors and deduction-guide-shaped declarations. By contrast, an over-broad rule that turns a valid construct into the wrong AST or creates uncontrolled ambiguity is not practical.

### Avoid unnecessary ambiguity

Ambiguity is justified only when the correct interpretation requires semantic facts that the syntax deliberately does not collect. Token spelling, delimiters, precedence, associativity, and other purely syntactic facts should select one shape whenever the standard supplies such a choice.

The dedicated `auto` form is the current historical-compatibility example. The preferred optional `else` and the longest `operator new[]`/`operator delete[]` forms are current token-only priority examples. Limited over-acceptance must not reintroduce alternatives in these cases merely because an older edition or an invalid spelling could be represented.

### Factor shared syntax before interpretations diverge

An identifier chain has the same token structure before lookup tells whether it names a type, value, namespace, or template. [`QualifiedName`](../../Test/Source/BuiltIn-Cpp/Syntax/Ast/QualifiedName.txt) therefore derives from `TypeOrExpr`. [`QualifiedName.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/QualifiedName.txt) builds that shared node once instead of manufacturing duplicate type and expression trees.

Create distinct AST nodes where syntax actually diverges, not merely where semantics might later assign different meanings. This keeps ambiguity local and avoids a Cartesian product of equivalent candidates.

### Preserve semantic ambiguity

The AST roots `TypeOrExprOrOthers`, `Declaration`, `TypeOrExpr`, and `Statement` are marked `@ambiguous` in [`Ast/Ast.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Ast/Ast.txt). Declarator-related nodes also expose ambiguity where competing constructions need a common result type.

The grammar should use token-only facts, precedence, and required punctuation. It should not guess whether a name denotes a type or template. For example, `A<B>C;` can remain both a declaration and an expression statement. BuiltIn-Cpp keeps those candidates unresolved; a separate consumer may inspect them if it already owns semantic information, but symbol resolution is not part of this grammar.

An ambiguity is constructible only when the alternatives have a suitable ambiguity-enabled common AST base and compatible assembly shapes. When adding a competing alternative, design its AST relationship at the same time as its syntax.

### Compose AST fragments instead of duplicating objects

Partial rules such as `_DeclaratorAKFirst` and `_ClassDeclPrefix` describe fragments of a larger object. Reuse clauses such as `!_BExpr1` pass an existing object through a wrapper level. These mechanisms keep one AST object aligned with one syntactic construct while allowing its syntax to be distributed across focused helpers.

Use an ordinary clause when a genuinely new node begins. Use a partial rule when several token forms fill fields of an object owned by the caller. Use a reuse clause when a routing or precedence rule should return the child object unchanged.

### Let outer contexts own delimiters

Comma-free expressions allow list rules to own commas. Declaration cores that need a semicolon leave it to `_DeclNeedSemicolon`; complete declarations selected by `_DeclRejectSemicolon` consume their own semicolon or body. First and subsequent declarators have separate wrappers because `VariablesDeclaration` stores the shared base type once, while comma tails contribute only additional declarator parts.

Delimiter ownership prevents a reusable rule from consuming punctuation that its caller needs for structure.

### Use priority only for syntax-only choices

The preferred optional form `+["else" _Stat:falseStat]` associates an `else` with the nearest eligible `if`. Similar preference in operator names chooses `operator new[]` over the shorter prefix when the brackets are present.

Priority is appropriate when the token sequence itself decides the preferred structure. It must not be used to select a type, template, declaration, or expression interpretation that depends on lookup.

## Context specialization with `allow_GT`

[`API.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/API.txt) declares the `allow_GT` switch. The expression precedence rule for the standalone relational `>` operator is conditional on that switch.

- `_Expr_Raw` and `_Expr_NoComma_Raw` expose the shared expression implementation without assigning `allow_GT`; they inherit the caller's switch value.
- `_Expr`, `_Expr_NoComma`, and `_TypeOrExpr` invoke their raw rules with `allow_GT` enabled.
- `_TypeOrExpr_NoComma_NoGT` invokes the same implementation with `allow_GT` disabled.
- [`Generic.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Generic.txt) uses the no-comma/no-`>` view for parameter defaults and template arguments, allowing the next `>` token to close the template list. Parameter declarations otherwise reuse the ordinary type and declarator rules.

This is a compile-time grammar specialization. Switch rewriting creates the required rule variants before automaton construction; there is no mutable parser mode at runtime. Parentheses can still contain an ordinary expression because nested rules explicitly re-enter the normal `_Expr` API.

This pattern is the preferred solution when one context excludes a small, precise portion of a shared category. Add a switch at the narrowest production that differs, expose raw endpoints, and keep all other grammar layers shared.

## Declarators: the central orthogonal subsystem

### Why base types and declarators are separate

[`Types.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Types.txt) owns fundamental types, qualified names used as types, and `const`/`volatile` around the base type. It deliberately does not enumerate pointer, reference, array, function, member-pointer, or nested combinations.

Those combinations belong to declarators. Conceptually:

```text
complete type-id = type-before-declarator + abstract declarator
declaration      = declaration keywords + type-before-declarator + named declarator + initializer/body
```

`_Type` in `API.txt` combines `_TypeBeforeDeclarator` with `_DeclaratorWithoutName`. Variable and function declarations combine the same `_TypeBeforeDeclarator` with a context-appropriate named declarator. This is why the type grammar and declaration grammar are highly connected without duplicating one another.

That organization follows the orthogonality of the C++ grammar: a decl-specifier sequence establishes the base, while a declarator describes how a name or abstract type is modified. The same declarator machinery must serve both type-ids and declarations.

Consider:

```C++
int *items[4];
int (*items)[4];
int (*factory(double))[4];
```

All three begin with the same base type. Their meanings differ only through the declarator's prefix operators, parenthesized nesting, function parameters, and array suffixes. Encoding the complete spellings as separate type productions would make those dimensions impossible to maintain independently.

### Declarator regions

[`DeclaratorComponents.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclaratorComponents.txt) defines the atoms. [`DeclaratorConfigurations.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclaratorConfigurations.txt) assembles them under different name policies.

A declarator has three structural regions:

1. **Before the inner declarator**: pointer/reference operators, CV qualifiers at the appropriate level, member-pointer qualifiers, alignment, declaration keywords, and calling-convention extensions.
2. **The inner declarator**: a name, operator name, pack marker, bit-field width, template arguments on the name, or a parenthesized nested declarator.
3. **After the inner declarator**: a function part or one or more array parts.

Parenthesized nesting changes which suffix binds to which prefix. The `innerDeclarator` field preserves that structure instead of flattening all modifiers into one list.

The `AK` partial rules are deliberately categorized:

- `_DeclaratorAKFirst` admits the components accepted by the current grammar at the start of a prefix sequence.
- `_DeclaratorAKFollow` admits subsequent components, including CV qualifiers.
- `_DeclaratorAKCV` supplies the CV-only boundary used before a nested declarator.
- `_DeclaratorAKMember` supplies the qualified-name-and-`::` fragment of a member pointer; a separate `*` component completes the pointer.
- `_DeclaratorAKCtorDtor` is a separate contract for the qualified prefix of an untyped function-shaped declarator, even though its current spelling matches the member-qualification category.

The current grammar keeps these semantic categories separate even though their productions are identical, because each public rule documents a different caller contract.

### Worked declarator examples

The declaration owns the base type; the declarator owns the binding around a name or an anonymous type-id. In the AST, one `Declarator` nesting level contains:

- `advancedTypes` for prefix components such as `*`, `&`, CV qualifiers, and member qualification;
- optional `id` or `innerDeclarator` for the core (abstract configurations may have neither);
- `funcPart` or `arrayParts` for suffixes;
- initialization outside the declarator, in `DeclaratorVariablePart`.

The binding effect of parentheses is preserved by `innerDeclarator`, although the delimiter tokens themselves are not stored as fields. That structural distinction separates an array of pointers from a pointer to an array.

| Source | Base owned by the outer declaration | Declarator shape | Resulting structure |
| --- | --- | --- | --- |
| `int value;` | `int` | name `value` | object of type `int` |
| `int * const pointer = nullptr;` | `int` | prefix `* const`, name `pointer` | const pointer to `int` |
| `int *items[4];` | `int` | prefix `*`, name `items`, suffix `[4]` | array of four pointers to `int` |
| `int (*items)[4];` | `int` | nested `(*items)`, outer suffix `[4]` | pointer to an array of four `int` values |
| `int (*callback)(double);` | `int` | nested `(*callback)`, outer function suffix | pointer to a function returning `int` |
| `int (*callbacks[8])(double);` | `int` | nested `(*callbacks[8])`, outer function suffix | array of eight pointers to functions returning `int` |
| `int Widget::*member;` | `int` | member qualification `Widget::`, `*`, name | pointer to an `int` member of `Widget` |
| `int (Widget::*method)(double) const;` | `int` | nested member pointer, outer function suffix | pointer to a const member function returning `int` |

#### Named object and pointer qualifiers

```C++
int value;
const int* pointerToConst;
int* const constPointer = nullptr;
```

All three declarations use the shared typed-variable machinery in `DeclarationVariable.txt` with one `_TypeBeforeDeclarator` and one `_DeclaratorRequiredName`. File/class declarations reach it through `_MultiVarsOrFuncDecl`, declaration statements through `_MultiVarsOrFuncForwardDecl`, and loop/selection contexts may call `_MultiVarsDecl` directly.

- `value` needs only `_DeclaratorName`.
- `pointerToConst` stores `const int` in the base type and `*` in the declarator.
- `constPointer` stores `int` in the base type and the ordered `* const` components in `advancedTypes`.

This separation matters: base-type CV and pointer CV must not be normalized into the same field. Adding a new pointer-like modifier belongs in the declarator components and automatically reaches declarations and abstract type-ids.

#### Array of pointers versus pointer to array

```C++
int *arrayOfPointers[4];
int (*pointerToArray)[4];
```

For `arrayOfPointers`, `_DeclaratorBeforeInner` consumes `*`, `_DeclaratorName` consumes the name, and `_DeclaratorAfterInner` consumes `[4]` through `_DeclaratorArrayPart`. The array suffix binds to the name at that level, with the pointer applying to each element type.

For `pointerToArray`, `_DeclaratorInnerRequiredName` first creates a parenthesized `innerDeclarator` for `*pointerToArray`. The outer declarator then receives `[4]`. Keeping the nesting produces the opposite binding without inventing a separate “pointer-to-array type” production.

Any change to the shared `_DeclaratorArrayPart` must be tested in both shapes. Adding an array feature only to a variable-declaration consumer would fail to reach the parenthesized type-id and function-return forms.

#### Functions, function pointers, and arrays of function pointers

```C++
int function(double argument);
int (*callback)(double);
int (*callbacks[8])(double);
auto makeCallback() -> int (*)(double);
```

`_DeclaratorFunctionPart` is the one function-suffix implementation in all four declarations. Its parameters reuse `_FunctionParameter`; its trailing return type reuses `_Type`.

- `function` has a name followed directly by the function part.
- `callback` puts `*callback` in `innerDeclarator`, then attaches the function part outside the parentheses.
- `callbacks` puts the array suffix inside that nested pointer declarator before attaching the outer function part.
- `makeCallback` uses an ordinary required-name declarator for the function and an abstract declarator inside the deferred `_Type` for `int (*)(double)`.

The fourth case is an important orthogonality check: a complex abstract type appears inside a complex named declaration, but both use the same declarator component rules.

#### Abstract declarators in type-ids

```C++
using Pointer = int*;
using PointerToArray = int (*)[4];
using Callback = int (*)(double);

sizeof(int[4]);
static_cast<int*>(value);
```

`_UsingTypeDecl`, `sizeof`, and named casts all reach the public `_Type` rule. `_Type` parses `_TypeBeforeDeclarator` and `_DeclaratorWithoutName`, so the same prefix, nesting, array, and function components are available with no identifier.

The anonymous equivalents line up with named declarations:

| Named declaration | Abstract type-id | Shared components |
| --- | --- | --- |
| `int *pointer` | `int*` | pointer prefix |
| `int array[4]` | `int[4]` | array suffix |
| `int (*pointer)[4]` | `int(*)[4]` | nested pointer plus outer array |
| `int (*callback)(double)` | `int(*)(double)` | nested pointer plus outer function part |

If a feature works only in the named or only in the abstract column, it was probably added to a consumer instead of the owning declarator component/configuration.

#### One base type with multiple declarators

```C++
int *left, right[4], (*callback)(double);
```

`VariablesDeclaration.type` stores `int` once. The no-semicolon variable core uses `_MultiVarsDeclVariablePartFirst` for `*left`; complete file/class declarations use `_MultiVarsOrTypedFuncDeclVariablePart` for that first item. Both routes hand `right[4]` and the recursive `(*callback)(double)` tail to `_MultiVarsDeclVariablePartSecond`. Each `DeclaratorVariablePart` has an independent declarator and initializer, so `*` from the first item cannot leak into later items.

This is why first and subsequent variable-part rules exist even though they share declarator machinery. A new modifier must work after a comma as well as on the first declarator; copying the whole declaration rule for later items would destroy that invariant.

#### Parameters reuse optional-name declarators

```C++
void visit(
    int,
    const char* name,
    double values[3],
    int (*callback)(double)
);
```

Every item is parsed by `_FunctionParameter`:

- `int` uses the type-only alternative.
- `const char* name` attaches an optional-name pointer declarator.
- `double values[3]` attaches an optional-name array declarator.
- `int (*callback)(double)` attaches an optional-name nested function-pointer declarator.

C++ later adjusts array and function parameters to pointer types, but the current syntax AST preserves the written array or function shape. BuiltIn-Cpp does not perform that semantic adjustment and does not duplicate parameter-only array or function syntax.

The same policy dimension produces related configurations for other contexts: `_CatchParameter` permits an optional name without a general initializer, `_ForEachParameter` requires a name and leaves `:` to the statement rule, and `_ExprOrVarCondition` requires an initialized declaration candidate.

#### Declarator structure can intentionally remain ambiguous

```C++
S function(int(a));
S object((int(a)));
S dependent(auto()->C);
```

The first spelling has the classic function-declaration preference. The extra parentheses in the second force an object initializer. The third remains unresolved because establishing whether `C` is a type suitable for the trailing return requires lookup.

The current parser builds candidates through the same type, expression, parameter, and declarator rules rather than special `S`/`int` productions. It preserves the declarator binding structure and leaves lookup-dependent cases unresolved. Syntax-only preference gaps are recorded in [De-ambiguation Improvements](Cases_Improvement.md); they are not implemented by this grammar today.

#### Typedef declarations and aliases reach the same type shape differently

```C++
typedef int (*CallbackA)(double);
using CallbackB = int (*)(double);
```

The `typedef` form uses `_MultiTypedefVarsDeclWithoutKeyword`: `int` is the shared base and `(*CallbackA)(double)` is a required-name declarator. The alias form uses `_UsingTypeDecl`: its right side is one `_Type` containing the nameless `(*)(double)` declarator. Both paths share declarator components, so function-pointer support is implemented once.

#### Untyped function-shaped declarators

```C++
struct Widget
{
    Widget(int value);
    ~Widget();
    operator bool() const;
};

template<typename T>
struct Box
{
    Box(T value);
};

Box(int) -> Box<int>;
```

These declarations have no `_TypeBeforeDeclarator`. `_DeclaratorUntypedFuncWithoutKeyword` combines an optional qualified prefix, `_DeclaratorUntypedFuncId`, and `_DeclaratorFunctionPart`.

- An ordinary name covers constructors and also the deduction-guide shape.
- `_DtorIdentifier` covers destructors.
- `_OperatorTypeIdentifier` covers conversion functions and reuses `_TypeWithoutFuncVar` for the conversion type.

The ordinary-name branch intentionally accepts a little more than strict C++ would permit in every scope. That bounded over-acceptance keeps one untyped function-shaped declarator path. The syntax AST leaves the ordinary-name form unclassified, and compiler-invalid uses are outside the indexer's responsibility.

#### Member pointers combine qualification, pointer, and nesting

```C++
int Widget::*member;
int (Widget::*method)(double) const;
```

In the current grammar, `_DeclaratorAKMember` contributes the `Widget::` qualification and a following pointer component contributes `*`. The data-member declaration can use those components directly. The member-function pointer puts `Widget::*method` in `innerDeclarator` and attaches `_DeclaratorFunctionPart` outside it, where `const` is a function qualifier.

The qualification and `*` are semantically one member-pointer operator, but the current grammar stores them as a member-qualification component followed by a separate pointer component. The resulting long-prefix ambiguity is recorded as a current gap in [De-ambiguation Improvements](Cases_Improvement.md).

#### Choosing a declarator configuration

| Context | Public entry | Name policy and restriction |
| --- | --- | --- |
| Complete type-id such as `int(*)[4]` | `_Type` → `_DeclaratorWithoutName` | no name; full suffix support |
| Conversion type in `operator int*()` | `_TypeWithoutFuncVar` → `_DeclaratorWithoutNameAndFuncVar` | no name; restricted direct suffixes |
| Variable or field `int *value` | `_DeclaratorRequiredName` | name required |
| Function parameter `int *value` or `int*` | `_DeclaratorOptionalName` | name optional |
| Catch parameter `const Error& error` | `_CatchParameter` → `_DeclaratorOptionalName` | name optional; no initializer |
| Range-for declaration `auto& item` | `_ForEachParameter` → required-name path | name required; `:` owned by statement |
| Constructor, destructor, conversion, or guide | `_DeclaratorUntypedFuncWithoutKeyword` | no base type; function part required |

When a new C++ feature changes declarators, test every applicable row. If two rows need different acceptance, introduce one clearly named configuration switch at that dimension while keeping the component grammar shared.

#### Declaration keywords participate in the shared declarator grammar

```C++
struct Owner
{
    friend int compare(const Owner&, const Owner&);
    friend class Helper;

    template<typename T>
    friend void visit(T&& value);
};
```

`_DeclarationKeywordWithoutFriend` accepts `extern` and an optional linkage string; `_DeclarationKeyword` adds `friend`; `_DeclaratorKeyword` then combines those declaration-wide words with storage, function, and calling-convention words. Declaration routers use the narrower views where class/enum forwarding or friend declarations need different routing, while typed variable/function paths use `_DeclaratorKeyword` before the base type.

The declarator prefix also admits `_DeclaratorKeyword`, so the current grammar accepts keyword orders such as `int static *method();` and records keywords at the syntactic level where they occur. It does not normalize one strict standard decl-specifier sequence. This is the implemented bounded over-acceptance: the same declarator combinators handle every position, while compiler-invalid arrangements are left for the compiler that validates input before the indexer sees it.

### Declarator configurations

| Public rule | Context contract |
| --- | --- |
| `_DeclaratorWithoutNameAndFuncVar` | Restricted abstract declarator used by conversion-function types; it excludes a direct outer function/array suffix. |
| `_DeclaratorWithoutName` | Full abstract declarator for a type-id. |
| `_DeclaratorOptionalName` | Declarator for parameters and similar contexts where the name may be absent. |
| `_DeclaratorRequiredName` | Declarator for variables, fields, bit-fields, and other contexts that require a name. |
| `_InnerDeclaratorInnerRequiredName` | Required-name form behind an inner CV boundary; also supports later comma-separated declarators. |
| `_DeclaratorUntypedFuncWithoutKeyword` | Function-shaped declarator with no base type; its name may be ordinary, a destructor, or a conversion-function identifier. |

Do not merge these rules merely because they share most productions. Required, optional, and absent names are independent context options, and the restricted conversion type has a different suffix contract.

### Reuse from declarations

[`DeclarationVariable.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclarationVariable.txt) adds initialization and declaration context around declarators:

- Function parameters use an optional name and permit a default `=` initializer.
- Catch parameters use an optional name but not a general initializer.
- Conditions require a named declarator and an initializer when the condition is a declaration.
- Range-for parameters require a name and leave the `:` delimiter to the statement rule.
- Ordinary variable declarations require a name and support all initializer forms.
- Constructors, destructors, conversion functions, and deduction-guide-like ordinary names use the untyped-function configuration.
- A comma tail reuses the shared base type but parses another declarator, as in `int *pointer, value;`.

When a declarator feature changes, first identify which structural region owns it. Then expose it through every configuration in which the standard permits it. Do not patch each declaration form independently.

## Responsibility boxes and public rules

The following catalog is the cross-file API of the current syntax. Private helpers are implementation details of their file and may be refactored without changing another box.

### `API.txt`

[`API.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/API.txt) owns canonical entry views, ambiguity unions, switch assignment, and the file parser root.

| Public rule | Responsibility |
| --- | --- |
| `_Type` | Complete type-id: a base type plus an optional full abstract declarator and allowed leading declarator keywords. Also a generated parser entry. |
| `_Expr_NoComma` | Canonical expression with `allow_GT` enabled and the comma operator excluded. |
| `_Expr` | Canonical full expression with `allow_GT` enabled. Also a generated parser entry. |
| `_Expr_Argument` | Comma-free expression with an optional pack-expansion ellipsis. |
| `_TypeOrExpr_NoComma_Raw` | Ambiguity union of a comma-free expression and a type that inherits the caller's switch values. |
| `_TypeOrExpr_Raw` | Ambiguity union of a full expression and a type that inherits the caller's switch values. |
| `_TypeOrExpr_NoComma_NoGT` | Template-list view of the comma-free type-or-expression union with `>` disabled. |
| `_TypeOrExpr` | Canonical ambiguity union of a full expression and a type. Also a generated parser entry. |

The private `_File` rule is a generated parser entry that assembles one or more complete declarations.

### `QualifiedName.txt`

[`QualifiedName.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/QualifiedName.txt) owns identifier forms and recursive qualified-name assembly.

| Public rule | Responsibility |
| --- | --- |
| `_NameIdentifier` | Ordinary identifier. |
| `_DtorIdentifier` | Destructor identifier beginning with `~`. |
| `_OperatorIdentifier` | Overloadable operator, allocation/deallocation, and literal-operator identifiers. |
| `_OperatorTypeIdentifier` | Conversion-function identifier of the form `operator type`. |
| `_QualifiedName` | Contextual/root names and recursive `::` chains, including template arguments, `auto`, `decltype`, and member tails. |
| `_QualifiedNameAfterTypename` | Dependent qualified name after `typename`, requiring at least one qualification step. |

### `Expressions.txt`

[`Expressions.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Expressions.txt) owns primary/postfix/prefix expressions and the complete precedence ladder.

| Public rule | Responsibility |
| --- | --- |
| `_PrimitiveExpr` | Literals, parenthesized and braced expressions, named casts, system queries, and lambdas. |
| `_BExpr_NoComma` | Expression ladder through logical-or, throw, conditional, and assignment, excluding comma. |
| `_BExpr` | Adds the comma operator above `_BExpr_NoComma`. |
| `_Expr_NoComma_Raw` | Comma-free expression endpoint that inherits the caller's switch values. |
| `_Expr_Raw` | Full-expression endpoint that inherits the caller's switch values. |

Each precedence level reuses the tighter level and uses direct left recursion for its left-associative operators. Add an operator at the level matching its binding and associativity instead of creating a special consumer rule.

### `Types.txt`

[`Types.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Types.txt) owns base-type spelling and the boundary before declarators.

| Public rule | Responsibility |
| --- | --- |
| `_PrimitiveType` | Fundamental type spellings and `typename`-forced qualified types. |
| `_CallConstructibleType` | Base-type subset usable directly as the operand of functional construction/cast syntax. |
| `_TypeBeforeDeclarator` | Base type and its outer CV qualifiers before a declarator is attached. |
| `_TypeWithoutFuncVar` | Restricted type used in conversion-function identifiers, with no direct outer function/array suffix. |

### `Statements.txt`

[`Statements.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Statements.txt) owns statement composition and declaration/expression ambiguity in statement positions.

| Public rule | Responsibility |
| --- | --- |
| `_BlockStat` | Compound statement; exported for lambdas, function bodies, constructors, and other declaration files. |

The private `_Stat` rule is a generated parser entry. Statement internals remain private because other files need blocks, not arbitrary fragments of the statement router.

### `Generic.txt`

[`Generic.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Generic.txt) owns template delimiters and their type-or-expression contents.

| Public rule | Responsibility |
| --- | --- |
| `_GenericHeader` | Template parameter list, including type, non-type, template, defaulted, and pack forms currently represented. |
| `_GenericArguments` | Template argument list containing ambiguity-preserving type-or-expression arguments and pack expansions. |

Both depend on the no-comma/no-`>` view instead of copying expression or type syntax.

### `DeclaratorComponents.txt`

[`DeclaratorComponents.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclaratorComponents.txt) owns declarator atoms and suffix objects.

| Public rule | Responsibility |
| --- | --- |
| `_DeclarationKeywordWithoutFriend` | Declaration keyword subset valid where `friend` is excluded; currently includes `extern` and optional language linkage. |
| `_DeclarationKeyword` | Adds `friend` to the declaration-keyword subset. |
| `_DeclaratorKeyword` | Storage, function, calling-convention, and extension keywords accepted around a declarator. |
| `_DeclaratorAKFirst` | Partial `Declarator` fragment accepted by the current grammar as the first prefix component. |
| `_DeclaratorAKFollow` | Partial fragment for subsequent prefix components, including CV. |
| `_DeclaratorAKCV` | CV-only fragment for an inner declarator boundary. |
| `_DeclaratorAKMember` | Member-qualification fragment ending in `::`; a separate `*` component completes a member pointer. |
| `_DeclaratorAKCtorDtor` | Qualified prefix contract for function-shaped declarators with no base type. |
| `_DeclaratorFunctionPart` | Parameters, function suffix keywords, and optional trailing return type. |
| `_DeclaratorFunctionPartOptionalParameters` | Lambda-oriented function suffix that may omit the explicit parameter list. |
| `_DeclaratorArrayPart` | One array suffix. |
| `_DeclaratorId` | Ordinary or operator-function declarator name. |
| `_DeclaratorUntypedFuncId` | Ordinary, destructor, or conversion-function identifier for a declaration with no base type. |

### `DeclaratorConfigurations.txt`

[`DeclaratorConfigurations.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclaratorConfigurations.txt) owns recursive assembly and context policies.

| Public rule | Responsibility |
| --- | --- |
| `_InnerDeclaratorInnerRequiredName` | Required-name declarator behind the nested inner boundary. |
| `_DeclaratorWithoutNameAndFuncVar` | Restricted nameless declarator for conversion types. |
| `_DeclaratorWithoutName` | Full abstract declarator. |
| `_DeclaratorOptionalName` | Full declarator with an optional name. |
| `_DeclaratorRequiredName` | Full declarator with a required name. |
| `_DeclaratorUntypedFuncWithoutKeyword` | Function-shaped declarator with no base type, using an ordinary, destructor, or conversion-function identifier. |

### `DeclarationVariable.txt`

[`DeclarationVariable.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclarationVariable.txt) combines types, declarators, initializers, bodies, and context-specific declaration policies.

| Public rule | Responsibility |
| --- | --- |
| `_VarCtorInit` | Parenthesized or braced initializer. |
| `_VarInit` | Any supported `=`, parenthesized, or braced initializer. |
| `_FunctionParameter` | Type-only or typed parameter with optional name and default. |
| `_CatchParameter` | Catch declaration with optional name. |
| `_ExprOrVarCondition` | Ambiguity-preserving expression or initialized declaration condition. |
| `_ForEachParameter` | Named range-for declaration without an initializer. |
| `_MultiTypedefVarsDeclWithoutKeyword` | Typedef-compatible declarator chain without the leading `typedef`. |
| `_MultiVarsDeclVariablePartSecond` | Recursive comma tail after a shared base type; also used after class and enum definitions. |
| `_MultiVarsDecl` | Optionally keyword-prefixed variable declaration core that leaves its semicolon to the caller. |
| `_MultiVarsOrFuncForwardDecl` | Variable/function declaration core that still needs the caller's semicolon. |
| `_MultiVarsOrFuncDecl` | Complete variable/function declaration that consumes a semicolon or function/constructor body. |

The words “forward” and “complete” mainly describe termination ownership here; they are not a complete semantic classification of C++ declarations.

### `DeclarationClasses.txt`

[`DeclarationClasses.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclarationClasses.txt) owns class-family declarations and recursively routes member declarations.

| Public rule | Responsibility |
| --- | --- |
| `_ClassDecl` | Class/struct/union definition, bases, access sections, body, and optional following declarators. |
| `_ClassForwardDecl` | Class/struct/union declaration without a body. |

### `DeclarationOthers.txt`

[`DeclarationOthers.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/DeclarationOthers.txt) owns declaration families that do not belong to the class or variable boxes.

| Public rule | Responsibility |
| --- | --- |
| `_StaticAssertDecl` | `static_assert` declaration without its outer semicolon. |
| `_TypedefDecl` | `typedef` around a compatible variable, class, or enum declaration. |
| `_ExternDeclRejectSemicolon` | Linkage block containing complete declarations. |
| `_NsDecl` | Unnamed, ordinary, or nested namespace definition. |
| `_UsingNsDecl` | Using-directive. |
| `_UsingValueDecl` | Using-declaration with an optional `typename` disambiguator. |
| `_UsingTypeDecl` | Type-alias declaration. |
| `_FriendTypeDecl` | Friend type declaration. |
| `_EnumDecl` | Scoped/unscoped enum definition and optional following declarators. |
| `_EnumForwardDecl` | Opaque scoped/unscoped enum declaration. |

### `Declarations.txt`

[`Declarations.txt`](../../Test/Source/BuiltIn-Cpp/Syntax/Syntax/Declarations.txt) is the declaration router and owns semicolon policy.

| Public rule | Responsibility |
| --- | --- |
| `_DeclNeedSemicolon` | Declaration core whose caller must consume `;`; used by declaration statements and recursively by templates. |
| `_DeclRejectSemicolon` | Complete file/class/namespace declaration that consumes its own terminator or body. |

These rules depend back on class, enum, namespace, and variable rules, while those boxes recursively call `_DeclRejectSemicolon` for bodies. This cycle is intentional and is exactly why the narrow public API boundaries matter.

## Dependency topology

The files separate responsibilities, not graph layers. The major supported cycles are:

- qualified names → types → declarators → qualified names;
- expressions → types/declarators/generics/statements → expressions;
- declarator components → declarator configurations → variable declarations → function parameters → declarator components;
- declaration routers → class/namespace declarations → declaration routers.

Do not break these cycles by cloning rules into “lower” files. The parser compiler resolves public forward references after registering all symbols. Break a cycle only when the concepts themselves have become unnecessarily coupled, not merely because the file graph is cyclic.

## Maintenance workflow

When adding or correcting syntax:

1. Identify the syntactic dimension that changes: token formation, name, expression, base type, declarator component, initializer, statement, or declaration routing.
2. Modify the one rule box that owns that dimension.
3. Route every consumer through the same canonical public rule. Add a restricted view only when a context excludes a precise option.
4. Keep new helpers private. Mark the narrowest stable rule `@public` only when another file must call it.
5. Check whether competing alternatives share an `@ambiguous` AST base and can build a local ambiguity node without symbol lookup.
6. Check delimiter ownership, precedence, associativity, declarator name policy, and semicolon ownership.
7. Cover the feature in every structurally distinct context it affects, especially type-id versus declaration, first versus later declarators, ordinary versus nested declarators, and template contexts with `>` disabled.
8. Update this guide only when the grammar's semantic contract or maintenance invariant changes; mechanical rewrites should not create documentation churn.

Use [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md) when the issue occurs before phase-7 parsing, the versioned case files for missing standard syntax, and [De-ambiguation Improvements](Cases_Improvement.md) for syntax-only postpasses and deliberately preserved ambiguity.
