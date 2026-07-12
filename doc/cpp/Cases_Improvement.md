# BuiltIn-Cpp De-ambiguation Improvements

## Feature and Case Index

- Qualified-Name Normalization
  - Case No.1 — Whitespace Around Scope Operators
  - Case No.2 — Complete Member-Pointer Qualifiers
  - Case No.3 — Elaborated and Dependent Disambiguators
- Standard Declaration Ambiguity
  - Case No.1 — Function Declarations
  - Case No.2 — Object Declarations
  - Case No.3 — Lookup-Dependent Trailing Return Type
  - Case No.4 — Parenthesized Parameter Types
- Statement Ambiguity
  - Case No.1 — Declaration Statements Forced by a Built-In Type
  - Case No.2 — Expression Statements
  - Case No.3 — Trailing Return Type Requiring Lookup
- Conditions Forced to Declarations
  - Case No.1
- Type-Id and Expression Ambiguity
  - Case No.1 — Template Arguments
  - Case No.2 — `sizeof` Operands
  - Case No.3 — Cast or Parenthesized Expression
  - Case No.4 — Structurally Shared Qualified Names
- Lookup-Dependent Ambiguities That Must Remain
  - Case No.1 — Declaration or Expression
  - Case No.2 — Template-Id or Relational Operators
  - Case No.3 — Call, Binary Expression, or Function Type
- AST Reclassification Without Lookup
  - Case No.1 — Deduction Guide
  - Case No.2 — Using-Enum Declaration
  - Case No.3 — Constrained or Non-Type Template Parameter
  - ~~Case No.4 — Comma-Less Ellipsis Classification [WON'T FIX]~~
- Language-Version Preference
  - Case No.1 — Multidimensional Subscript or Comma Expression
- Deliberate Practical Over-Acceptance
  - Case No.1 — Split Comparison and Shift Punctuators
  - Case No.2 — Split Operator Names
  - ~~Case No.3 — Longest New-Type-Id Boundary [WON'T FIX]~~
  - Case No.4 — Declarator Keywords as Orthogonal Components
- C++26 Reflection Boundary

This document describes improvements around the C++ syntax and AST produced by [`Test/Source/BuiltIn-Cpp`](../../Test/Source/BuiltIn-Cpp). It does not propose changes to the parser generator. The examples are a design corpus for manual AST postpasses that run after parsing.

This audit applies the priorities in [C++ Syntax Implementation Philosophy](Philosophy.md). Some names are deliberately undeclared. BuiltIn-Cpp preserves ambiguity instead of performing symbol resolution, so a case does not need to be a complete compiler test. The input reaching the indexer is assumed to have already been accepted by a compiler.

Implementation notes use two categories:

- **Additive** means the required normalization, classification, or AST identity is absent and can be added at its owning post-parse layer without reorganizing the grammar.
- **Structural** means the shared AST or grammar contract must change. The recommendation generalizes the owning rule family so every context benefits instead of patching one spelling.

Both categories may use bounded practical over-acceptance: accept a small, documented invalid superset when that choice substantially improves orthogonality and cannot misinterpret compiler-verified valid input.

## Expected Outcomes

| Outcome | Required behavior |
| --- | --- |
| **One interpretation** | A syntax-only postpass can retain one AST candidate without name lookup. |
| **Reclassify** | The current grammar accepts the tokens through an older or more general node; a postpass gives the construct its standard identity. |
| **Preserve ambiguity** | The choice requires name, type, template, or pack information. Keep an ambiguity node or an equivalently lossless representation. |
| **Practical over-acceptance** | The spelling is not a well-formed standard program, but accepting one useful interpretation cannot misread any valid compiler-verified input. |

## Recommended Pass Order

1. Normalize token chains whose shape is independent of lookup, especially qualified names and deliberately split punctuators.
2. Apply the standard's syntax-only declaration and type-id ambiguity rules.
3. Reclassify generic nodes whose token shape identifies a newer standard construct.
4. Preserve every choice that still depends on lookup or pack binding; for identical cross-version spellings, prefer the new-standard representation.
5. Publish every remaining lookup-dependent ambiguity node as part of the final index result.

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

The qualifier of the first declaration is `::qname_fixture::A::b::c::`; it must not become several nested declarators. The second declaration is the exact regression shape from [`TODO.md`](../../TODO.md). BuiltIn-Cpp intentionally does not decide whether those names denote namespaces and classes.

**Implementation suggestion — Additive:** Add a regression assertion first: the existing `_AdvancedTypeMember` already consumes one complete `_QualifiedName` plus the final `::`, and the following shared `_AdvancedTypeNoCVNoMember` component consumes `*`. Keep those orthogonal components so every declarator configuration continues to share the same qualified-name and pointer rules. If the GLR result contains equivalent prefix splits, deduplicate only structurally identical modifiers covering the same source range; do not create a member-pointer-only copy of qualified-name or pointer syntax.

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

**Implementation suggestion — Additive:** Keep the current shared identifier and `_QualifiedName` component chain. `_TypeIdentifier`, `_QualifiedNameAfterTypename`, and the `template` qualified-name fragment already apply the three syntactic restrictions without symbol lookup. Add regression coverage and, only if the index needs the written disambiguator, retain its token or source range on the existing name/type node. Do not add context-specific copies or wrappers around qualified-name recursion.

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

**Expected:** Preserve ambiguity because deciding whether `C` is a type requires lookup, which BuiltIn-Cpp does not perform.

```C++
S function_case(auto()->C);
```

Leading `auto` enables the function-declaration candidate, but it does not establish that `C` is a type.

**Implementation suggestion — Additive:** Let the declarator-ambiguity pass tag this node with a focused unresolved reason such as `TrailingReturnTypeNeedsLookup` and retain both candidates as the final index representation. This is post-parse metadata, not a grammar or symbol-resolution decision.

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

- `nested_type_parameter` preserves ambiguity because the standard's pointer-to-function interpretation depends on lookup establishing that `C` is a type. BuiltIn-Cpp does not perform that lookup.
- `nested_type_parameter_control` has one syntactic interpretation and explicitly spells the selected function-pointer declarator.
- `nested_array_parameter` preserves ambiguity because treating and adjusting `C[10]` as a function parameter depends on `C` being a type.

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

The final line is an expression even if `M` denotes a type, because its outermost declarator candidate does not begin with `auto`.

**Implementation suggestion — Additive:** Add the complementary syntax-only filters to the statement-ambiguity pass: member access, postfix operators, comma-expression arguments, and a trailing-return-shaped suffix without leading `auto` eliminate the declaration candidate. Keep the original source/candidate record for diagnostics, but return the one surviving expression statement.

### Case No.3 — Trailing Return Type Requiring Lookup

**Expected:** Preserve ambiguity because determining whether `M` is a type requires lookup, which BuiltIn-Cpp does not perform.

```C++
auto(s)()->M;
```

**Implementation suggestion — Additive:** Preserve this `StatementToResolve` in the final index and attach the same trailing-return lookup reason used for declarations. BuiltIn-Cpp does not collapse it by deciding whether `M` is a type; the statement grammar already owns the correct declaration/expression union.

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

**Implementation suggestion — Additive:** Canonicalize candidates that contain the same normalized `QualifiedName` node and source range into one spelling object with deferred semantic roles. The ambiguity layer should distinguish different syntax trees, not duplicate a structurally identical name merely because it may denote a type or value.

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

**Implementation suggestion — Additive:** Add an ambiguity-audit pass that records the unavailable lookup facts required by each candidate—type-name, template-name, or value-name—without selecting one. It should flatten nested `ToResolve` wrappers, deduplicate equivalent candidates, and publish the declaration/expression alternatives in the final index.

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

**Implementation suggestion — Additive:** Keep all call, binary-expression, and function-type candidates in one local ambiguity node after structural deduplication. The audit pass should verify that each candidate covers the same source range, expose its unavailable lookup facts, and publish the unresolved node without a selection step.

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

### Case No.4 — Comma-Less Ellipsis Classification [WON'T FIX]

```C++
template<typename T>
void dependent_pack_or_varargs(T...);

template<typename... Types>
void known_pack_parameter(Types...);

void known_varargs_parameter(int...);
void placeholder_pack_parameter(auto...);
```

A binder-aware postpass could classify these by resolving the surrounding parameter declarations:

- `T` is declared as a non-pack, so the first ellipsis is the deprecated C-style varargs suffix.
- `Types` is a declared type pack, so the second declaration has a function parameter pack.
- A built-in type cannot be a pack pattern, so `int...` is varargs.
- A placeholder followed by an ellipsis declares a parameter pack.

**Reason for not fixing:** Even a binder-local pass is symbol resolution, which BuiltIn-Cpp deliberately does not introduce. Keep the ellipsis and its source range in the shared declarator representation as the final index result; neither the syntax nor a mandatory syntax postpass chooses among pack expansion, pack declaration, and C-style varargs.

## Language-Version Preference

### Case No.1 — Multidimensional Subscript or Comma Expression

**Expected:** One lossless, new-standard-preferred index representation: an ordered subscript argument list retaining the original source range.

```C++
matrix[row, column];
```

In C++23 this is a two-argument subscript. An earlier edition treats it as a single comma-expression index, but producing both AST candidates would add language-version ambiguity without helping the symbol-blind index.

**Implementation suggestion — Structural:** Change `IndexExpr` in `Ast/Expressions.txt` from one `index` field to an ordered argument list and retain the complete source range. Update the single `_PostfixUnaryExpr` index rule to parse comma-separated `_Expr_Argument` values. Prefer this C++23 shape for every edition; the same node still indexes every operand in compiler-verified historical source, and no version-aware grammar or postpass is required.

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

**Implementation suggestion — Additive:** Extend `_OperatorIdentifier` with the same split `>` `>` and `<` `<` sequences already used by expression rules and map them directly to `Operators::RightShift` and `Operators::LeftShift`. Ignoring adjacency is the deliberate bounded over-acceptance. Keep the tokens separate globally because the same sequence closes nested template arguments; no spelling-provenance AST family is needed for indexing.

### Case No.3 — Longest New-Type-Id Boundary [WON'T FIX]

```C++
new int * i;
```

The standard consumes `*` as the pointer new-declarator and then rejects the stray `i`; it does not parse `(new int) * i`. Accepting the multiplication-shaped overparse is tolerable under the practical policy because the complete standard input is already invalid.

**Reason for not fixing:** The example is invalid and the accepted multiplication-shaped interpretation cannot misread compiler-verified input. [Complete New-Expression Type Syntax](Cases_17.md#complete-new-expression-type-syntax) separately adds valid allocated types by reusing shared type/declarator components in a new-expression view. That view must not gain extra machinery solely to reject this invalid boundary. Bounded over-acceptance is the intended outcome.

### Case No.4 — Declarator Keywords as Orthogonal Components

**Expected:** Accept both the valid declaration and the deliberately over-accepted alias type.

```C++
int static * method();
using X = int static *;
```

The second declaration is not standard C++, but it is harmless for compiler-verified input and follows from sharing the same declarator combinators everywhere a type is needed.

**Implementation suggestion — Additive:** Preserve `_DeclaratorKeyword` as a shared declarator component before and after the base type, as the current `_Type` and declarator configurations already do. Add regression coverage rather than distributing keyword-order matrices across declarations, type aliases, parameters, and nested declarators. A real compiler remains responsible for rejecting invalid placements.

## C++26 Reflection Boundary

Reflection has its own syntax-only longest-operand rule. Its valid boundary pairs and the deliberately excluded invalid form are documented in [C++26 Missing Features](Cases_26.md) under **Longest Reflect-Expression Boundary**.

**Implementation suggestion — Structural:** Put the longest-operand policy in the shared reflection-expression rule proposed by the C++26 gap document, with an explicit operand category and source boundary. The post-parse normalizer may discard shorter candidates only according to that syntax rule; it should not add reflection-specific exceptions to general prefix-expression ambiguity handling.
