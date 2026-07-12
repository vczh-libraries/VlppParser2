# BuiltIn-Cpp De-ambiguation Improvements

This document describes improvements around the C++ syntax and AST produced by [`Test/Source/BuiltIn-Cpp`](../../Test/Source/BuiltIn-Cpp). It does not propose changes to the parser generator. The examples are a design corpus for manual AST postpasses that run after parsing.

Some names are deliberately undeclared. BuiltIn-Cpp is expected to preserve ambiguity instead of performing symbol resolution, so a case does not need to be a complete compiler test. The input reaching the indexer is assumed to have already been accepted by a compiler.

Implementation notes use two categories:

- **Additive** means the required normalization, classification, or AST identity is absent and can be added at its owning post-parse layer without reorganizing the grammar.
- **Structural** means the shared AST or grammar contract must change. The recommendation generalizes the owning rule family so every context benefits instead of patching one spelling.

Both categories may use bounded practical over-acceptance: accept a small, documented invalid superset when that choice substantially improves orthogonality and cannot misinterpret compiler-verified valid input.

## Expected Outcomes

| Outcome | Required behavior |
| --- | --- |
| **One interpretation** | A syntax-only postpass can retain one AST candidate without name lookup. |
| **Reclassify** | The current grammar accepts the tokens through an older or more general node; a postpass gives the construct its standard identity. |
| **Preserve ambiguity** | The choice requires name, type, template, pack, or language-version information. Keep an ambiguity node or an equivalently lossless representation. |
| **Practical over-acceptance** | The spelling is not a well-formed standard program, but accepting one useful interpretation cannot misread any valid compiler-verified input. |

## Recommended Pass Order

1. Normalize token chains whose shape is independent of lookup, especially qualified names and deliberately split punctuators.
2. Apply the standard's syntax-only declaration and type-id ambiguity rules.
3. Reclassify generic nodes whose token shape identifies a newer standard construct.
4. Preserve every choice that still depends on lookup, pack binding, or the selected language version.
5. Let a later symbol-aware indexer pass resolve the remaining ambiguity nodes.

This order prevents an early semantic guess from destroying a valid parse candidate.

## Qualified-Name Normalization

Whitespace and comments do not break a `::` chain. Qualified-name assembly should operate on tokens and source ranges rather than textual adjacency.

### Case No.1 — Whitespace Around Scope Operators

**Expected:** One interpretation. Each statement contains one qualified name.

```C++
A ::B;
qname_fixture::A ::B;
qname_fixture::A:: B;
qname_fixture /* comment */ :: A /* comment */ :: B;
::qname_fixture::A::B;
```

The first line is the exact `A ::B` regression shape. Treating whitespace as a name boundary would incorrectly split every form in this case.

**Implementation suggestion — Additive:** First add a regression assertion that this input already produces one `_QualifiedName` parent chain; discarded whitespace/comments do not change the `ID SCOPE ID` token sequence. If duplicate candidates are actually observed, let the existing ambiguity-cleanup layer deduplicate structurally equal names only within that source occurrence while preserving its `codeRange`. No rule rewrite, punctuation reconstruction, or cross-occurrence AST interning is needed.

### Case No.2 — Complete Member-Pointer Qualifiers

**Expected:** One interpretation. The whole nested-name-specifier belongs to the member-pointer operator.

```C++
int ::qname_fixture::A::b::c::* canonical_member_pointer;
int ::a::b::c::* todo_member_pointer;
```

The qualifier of the first declaration is `::qname_fixture::A::b::c::`; it must not become several nested declarators. The second declaration is the exact regression shape from [`TODO.md`](../../TODO.md). Whether those names actually denote namespaces and classes is a later lookup question.

**Implementation suggestion — Structural:** Replace the loose `_DeclaratorAKMember` plus later `*` composition with one public member-pointer component that consumes a complete `_QualifiedName`, the final `::`, and `*` as one declarator modifier. Reuse that component from every declarator configuration. This gives the grammar one boundary for `::a::b::c::*` and prevents prefix splits without hard-coding qualification depths.

### Case No.3 — Elaborated and Dependent Disambiguators

**Expected:** One interpretation. The keywords already state how the following qualified name is used.

```C++
class qname_fixture::A::b::c* elaborated_type_pointer;

template<typename T>
void explicit_disambiguators(T& object)
{
    typename T::value_type* value;
    typename T::template rebind<int>* rebound;
    object.template call<int>();
}
```

`typename` forces a type interpretation and `template` forces a template-id interpretation. Preserve both tokens, or at least their source ranges, so the decision remains explainable in the AST.

**Implementation suggestion — Structural:** Give qualified-name use sites an orthogonal context wrapper carrying elaborated-type, `typename`, and `template` disambiguators while reusing the same `_QualifiedName` component chain. Extend `QualifiedName.txt` with the narrow public wrappers and add the wrapper/flags to the AST; do not create separate copies of qualified-name recursion for types and expressions.

## Standard Declaration Ambiguity

C++ contains declaration/expression ambiguities with rules defined in `[dcl.ambig.res]`. These rules should be applied by a focused AST pass rather than scattered through grammar productions.

### Case No.1 — Function Declarations

**Expected:** One interpretation: both are function declarations.

```C++
S v(int(a));
S w(int());
```

**Implementation suggestion — Additive:** Add a declarator-ambiguity pass over `VariablesDeclaration` candidates. Normalize each `Declarator` into a binding tree and apply `[dcl.ambig.res]` to select the function-declaration candidate when a parameter-shaped reading is available. Keep this policy in one pass shared by namespace, class, and block declarations.

### Case No.2 — Object Declarations

**Expected:** One interpretation: each declaration introduces an object.

```C++
S x((int(a)));
S y((int)a);
S z = int(a);
S object_case(B()->C);
```

The extra parentheses or initializer form prevent the first three from being function declarations. The potential parameter declaration in the last line has a trailing return type but does not begin with `auto`, so it remains an object initializer independently of lookup.

**Implementation suggestion — Additive:** Extend the same declarator-ambiguity pass with syntax-only object predicates: extra grouping, explicit initializer form, and a trailing-return-shaped expression without leading `auto`. The pass should select among already built candidates; it should not add special alternatives to `_MultiVarsOrFuncDecl` for each example.

### Case No.3 — Lookup-Dependent Trailing Return Type

**Expected:** Preserve ambiguity until `C` is known to be a type.

```C++
S function_case(auto()->C);
```

Leading `auto` enables the function-declaration candidate, but it does not establish that `C` is a type.

**Implementation suggestion — Additive:** Let the declarator-ambiguity pass tag this node with a focused unresolved reason such as `TrailingReturnTypeNeedsLookup` and retain both candidates. This is new post-parse metadata, not a grammar decision; the later symbol resolver should revisit the node after classifying `C`.

### Case No.4 — Parenthesized Parameter Types

```C++
namespace nested_parameter_fixture
{
    class C
    {
    };

    void nested_type_parameter(int(C));
    void nested_type_parameter_control(int(*fp)(C));
    void nested_array_parameter(int *(C[10]));
}
```

The expected outcomes are:

- `nested_type_parameter` preserves ambiguity until lookup establishes that `C` is a type. The standard interpretation is then a parameter of pointer-to-function type, not a parameter named `C` with redundant parentheses.
- `nested_type_parameter_control` has one syntactic interpretation and explicitly spells the selected function-pointer declarator.
- `nested_array_parameter` preserves ambiguity until `C` is known to be a type. Its selected function parameter is adjusted from `C[10]`.

**Implementation suggestion — Additive:** Add a canonical declarator binding-tree view that represents prefix operators, nested declarators, and function/array suffixes in binding order. Build it from the existing `Declarator` AST once, then run parameter adjustment and `[dcl.ambig.res]` against that view. This avoids teaching individual declaration rules how to reinterpret `int(C)`, `int(*fp)(C)`, and `int *(C[10])` separately without reorganizing the parsed AST.

## Statement Ambiguity

Statement-level declaration preference is not a blanket rule. Built-in type tokens and the trailing-return-type exception make several choices syntax-only, while other choices still require lookup.

### Case No.1 — Declaration Statements Forced by a Built-In Type

**Expected:** One interpretation for every line.

```C++
int(a);
int(*b)();
int(c) = 7;
int(d), e, f = 3;
int(g)(h, 2);
int(indexed)[5];
```

**Implementation suggestion — Additive:** Add a statement-ambiguity pass that recognizes declarations whose base begins with `_PrimitiveType` and selects the declaration candidate before any lookup. Reuse the same normalized declarator binding tree as declaration ambiguity; do not encode built-in-type preferences in each statement production.

### Case No.2 — Expression Statements

**Expected:** One interpretation for every line.

```C++
int(expression1)->member = 7;
int(expression2)++;
int(expression3, 5) << value;
S(s)()->M;
```

The final line is an expression even if `M` later resolves to a type, because its outermost declarator candidate does not begin with `auto`.

**Implementation suggestion — Additive:** Add the complementary syntax-only filters to the statement-ambiguity pass: member access, postfix operators, comma-expression arguments, and a trailing-return-shaped suffix without leading `auto` eliminate the declaration candidate. Keep the original source/candidate record for diagnostics, but return the one surviving expression statement.

### Case No.3 — Trailing Return Type Requiring Lookup

**Expected:** Preserve ambiguity until lookup determines whether `M` is a type.

```C++
auto(s)()->M;
```

**Implementation suggestion — Additive:** Preserve this `StatementToResolve` and attach the same trailing-return lookup reason used for declarations. The resolver should collapse it only after deciding whether `M` is a type; the statement grammar already owns the correct declaration/expression union.

## Conditions Forced to Declarations

The built-in type makes these condition-declarations syntax-only choices.

### Case No.1

**Expected:** One interpretation for both conditions.

```C++
if (int(if_value) = 0)
{
}

while (int(while_value) = 0)
{
}
```

**Implementation suggestion — Additive:** Reuse one condition-resolution pass for `if`, `while`, `switch`, and any future condition consumer. When the declaration candidate begins with a primitive type and contains the required initializer, retain it and discard the expression candidate. `_ExprOrVarCondition` should remain the single grammar entry for all condition forms.

## Type-Id and Expression Ambiguity

The syntax-only rules distinguish some type-id shapes from expressions. Other spellings share the same qualified-name structure and do not benefit from duplicate AST trees.

### Case No.1 — Template Arguments

**Expected:** The first argument is a type-id and the second is an expression.

```C++
X<int()> type_id_template_argument;
X<int(1)> expression_template_argument;
```

**Implementation suggestion — Additive:** Add a type-or-expression classifier that consumes the normalized declarator binding tree. A nameless function declarator selects the type-id candidate; a call with an expression argument selects the expression candidate. Run it uniformly on `_GenericArgument` rather than adding template-specific type productions.

### Case No.2 — `sizeof` Operands

```C++
sizeof(int());
sizeof(int(unsigned(a)));
sizeof(int(a));
```

The expected outcomes are:

- `sizeof(int())` and `sizeof(int(unsigned(a)))` select the type-id interpretation. Whether the resulting function type is semantically permitted is outside this pass.
- `sizeof(int(a))` selects the expression interpretation because a named declarator cannot occur in that type-id.

**Implementation suggestion — Additive:** Route the `sizeof` candidate pair through the same type-or-expression classifier used by template arguments. The `SizeofExpr` node should retain its operand source range and selected syntactic category; no `sizeof`-specific declarator grammar is needed.

### Case No.3 — Cast or Parenthesized Expression

```C++
(int()) + 1;
(int(unsigned(a))) + 1;
(int(a)) + 1;
```

The first two operands select the type-id interpretation. `int(a)` selects the expression interpretation.

**Implementation suggestion — Additive:** Apply the shared type-or-expression classifier before deciding whether the parenthesized prefix is a cast. Once the inner candidate is classified, retain the cast or ordinary parenthesized-expression branch. Keep this in the post-parse pass so `_AllPrefixUnaryExpr` and `_PrimitiveExpr` remain orthogonal consumers of `_Type` and `_Expr`.

### Case No.4 — Structurally Shared Qualified Names

**Expected:** Retain one lossless spelling tree and defer its semantic category; an ambiguity node containing duplicate qualified-name structures adds no information.

```C++
sizeof(T);
typeid(T);
X<T> value;
```

**Implementation suggestion — Additive:** Canonicalize candidates that contain the same normalized `QualifiedName` node and source range into one spelling object with deferred semantic roles. The ambiguity layer should distinguish different syntax trees, not duplicate a structurally identical name merely because it can later denote a type or value.

## Lookup-Dependent Ambiguities That Must Remain

These are the core cases where a symbol-blind parser should intentionally produce ambiguity. A postpass must not imitate compiler lookup by guessing that an identifier is a type or template.

### Case No.1 — Declaration or Expression

**Expected:** Preserve ambiguity.

```C++
A<B>C;
T * pointer_or_product;
T(value);
T(value) = other;
T(indexed)[5];
(T)(value);
```

`A<B>C;` can be a declaration or a relational-expression statement. The other lines similarly depend on whether `T` denotes a type.

**Implementation suggestion — Additive:** Add an ambiguity-audit pass that records the lookup facts required by each candidate—type-name, template-name, or value-name—without selecting one. It should flatten nested `ToResolve` wrappers and deduplicate equivalent candidates while preserving the declaration/expression alternatives for the symbol-aware resolver.

### Case No.2 — Template-Id or Relational Operators

**Expected:** Preserve ambiguity.

```C++
A<B>::C;
Name<a < b>;
```

**Implementation suggestion — Additive:** Preserve a shared token/name prefix and annotate the alternatives with `TemplateNameNeedsLookup` versus relational-expression requirements. Do not give template syntax priority over operators in `_QualifiedName` or `_BExpr5`; lookup is the only valid selector.

### Case No.3 — Call, Binary Expression, or Function Type

**Expected:** Preserve all candidates already represented by BuiltIn-Cpp coverage.

```C++
A<B>(C);
```

**Implementation suggestion — Additive:** Keep all call, binary-expression, and function-type candidates in one local ambiguity node after structural deduplication. The audit pass should verify that each candidate covers the same source range and expose its required lookup facts, leaving selection to later semantic processing.

## AST Reclassification Without Lookup

Some newer constructs already pass through older generic productions. Their explicit tokens and surrounding shape make a dedicated AST identity possible without resolving symbols.

### Case No.1 — Deduction Guide

**Expected:** Reclassify the generic function-declaration node as a deduction guide.

```C++
template<typename T>
struct GuideBox
{
    GuideBox(T);
};

template<typename T>
GuideBox(T) -> GuideBox<T>;
```

The arrow and matching template-name shape identify the construct.

**Implementation suggestion — Additive:** Add a `DeductionGuideDeclaration` AST class in `Ast/DeclsFuncVar.txt` and a post-parse reclassifier that recognizes the untyped function-shaped declaration plus trailing return type. Reuse the existing generic header, declarator function part, and qualified-name nodes; do not add a parallel guide-specific declarator grammar.

### Case No.2 — Using-Enum Declaration

**Expected:** Reclassify the current using-value node as a using-enum declaration.

```C++
enum class Color
{
    red,
    green,
};

using enum Color;
```

The explicit `enum` token makes lookup unnecessary.

**Implementation suggestion — Additive:** Add `UsingEnumDeclaration` beside the other using declarations in `Ast/Decls.txt`, then reclassify currently accepted forms when the target's `NameIdentifier.kind` is `Enum`; the AST already retains the discriminator, so no raw-token reconstruction is needed. Keep `_QualifiedName` as the one target-name rule. This postpass does not cover the currently rejected root-qualified spelling `using enum ::Color;`, whose structural grammar extension remains in [C++20 Missing Features](Cases_20.md).

### Case No.3 — Constrained or Non-Type Template Parameter

**Expected:** Preserve ambiguity. Unlike the previous cases, only lookup can determine whether `C` is a concept or a type.

```C++
template<C T>
struct constrained_or_nontype_parameter;
```

**Implementation suggestion — Structural:** Generalize the generic-parameter AST into ambiguity-enabled parameter kinds sharing one spelling range: type parameter, non-type parameter, constrained type parameter, and constrained placeholder. Add a constraint-prefix rule in `Generic.txt` that reuses `_QualifiedName`/generic arguments and combines with the existing declarator-based non-type parameter path. Preserve both candidates for `C T`; do not decide that `C` is a concept in the grammar.

### Case No.4 — Comma-Less Ellipsis Classification

```C++
template<typename T>
void dependent_pack_or_varargs(T...);

template<typename... Types>
void known_pack_parameter(Types...);

void known_varargs_parameter(int...);
void placeholder_pack_parameter(auto...);
```

A local binder-aware postpass can classify these without global symbol lookup:

- `T` is declared as a non-pack, so the first ellipsis is the deprecated C-style varargs suffix.
- `Types` is a declared type pack, so the second declaration has a function parameter pack.
- A built-in type cannot be a pack pattern, so `int...` is varargs.
- A placeholder followed by an ellipsis declares a parameter pack.

**Implementation suggestion — Additive:** Add a binder-local declarator classification pass that records template and abbreviated-function parameters before visiting their function declarators. It can then label an existing ellipsis as pack expansion, pack declaration, or C-style varargs without global lookup. Store the classification on `DeclaratorFunctionPart` rather than creating separate parameter-list grammars.

## Language-Version Ambiguity

### Case No.1 — Multidimensional Subscript or Comma Expression

**Expected:** Preserve the versioned alternatives, or use one lossless index representation that retains the original comma grouping.

```C++
matrix[row, column];
```

In C++23 this can be a two-argument subscript. In an earlier edition it can be a single comma-expression index.

**Implementation suggestion — Structural:** Change `IndexExpr` in `Ast/Expressions.txt` from one `index` field to an ordered argument list plus enough grouping/source information to reconstruct a legacy comma expression. Update the single `_PostfixUnaryExpr` index rule to parse comma-separated `_Expr_Argument` values and let a version-aware postpass expose either the C++23 multidimensional form or the earlier one-expression view. This keeps one index grammar instead of edition-specific branches.

## Deliberate Practical Over-Acceptance

The indexer only receives compiler-verified source, so accepting an additional invalid spelling is harmless when it cannot change the interpretation of valid code.

### Case No.1 — Split Comparison and Shift Punctuators

**Expected:** Normalize each split form to the same useful operator node as its adjacent spelling.

```C++
left >> right;
left > > right;

left << right;
left < < right;

left >= right;
left > = right;

left <= right;
left < = right;

left >>= right;
left > > = right;

left <<= right;
left < < = right;
```

For example, accepting `a > > b` as a right shift is practically correct even though the standard tokenization differs.

**Implementation suggestion — Additive:** No acceptance or operator-enum normalization work is required: the existing expression clauses already map both adjacent and split token sequences directly to the same `Operators` values. Only if spelling fidelity becomes necessary, add operator-local provenance at AST assembly so the original token slice is retained without changing precedence rules.

### Case No.2 — Split Operator Names

**Expected:** Apply the same normalization to operator-function identifiers.

```C++
struct PracticalOperatorNames
{
    PracticalOperatorNames operator > >(int);
    PracticalOperatorNames operator < <(int);
};
```

**Implementation suggestion — Structural:** Add a shared `OperatorSpelling` AST/rule family carrying the canonical `Operators` value and original token slice, and let both binary-expression nodes and `_OperatorIdentifier` reference it. Shift spellings can then consume `>` `>` or `<` `<` only where an operator is expected, deliberately ignoring adjacency. Do not combine these tokens globally before parsing, because the same sequence closes nested template argument lists; the shared operator-position rule provides orthogonality without changing valid template parses.

### Case No.3 — Longest New-Type-Id Boundary

```C++
new int * i;
```

The standard consumes `*` as the pointer new-declarator and then rejects the stray `i`; it does not parse `(new int) * i`. Accepting the multiplication-shaped overparse is tolerable under the practical policy because the complete standard input is already invalid.

**Implementation suggestion — Structural:** Replace `_NewExpr`'s `_QualifiedName` plus ad hoc array loop with an orthogonal new-type-id subsystem. Reuse `_TypeBeforeDeclarator`, factor a nameless `_DeclaratorForNewTypeId` from the declarator components with exactly the pointer/member-pointer and array forms allowed after `new`, and store the result as one type/declarator object in `NewExpr`. The new-expression rule should consume the longest legal new-type-id before parsing its initializer. This fixes ordinary `new int`, complex allocated types, and the `new int * i` boundary together instead of adding primitive-type and pointer special cases.

## C++26 Reflection Boundary

Reflection has its own syntax-only longest-operand rule. Its valid boundary pairs and the deliberately excluded invalid form are documented in [C++26 Missing Features](Cases_26.md) under **Longest Reflect-Expression Boundary**.

**Implementation suggestion — Structural:** Put the longest-operand policy in the shared reflection-expression rule proposed by the C++26 gap document, with an explicit operand category and source boundary. The post-parse normalizer may discard shorter candidates only according to that syntax rule; it should not add reflection-specific exceptions to general prefix-expression ambiguity handling.
