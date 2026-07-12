# BuiltIn-Cpp De-ambiguation Improvements

This document describes improvements around the C++ syntax and AST produced by [`Test/Source/BuiltIn-Cpp`](./Test/Source/BuiltIn-Cpp). It does not propose changes to the parser generator. The examples are a design corpus for manual AST postpasses that run after parsing.

Some names are deliberately undeclared. BuiltIn-Cpp is expected to preserve ambiguity instead of performing symbol resolution, so a case does not need to be a complete compiler test. The input reaching the indexer is assumed to have already been accepted by a compiler.

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

### Case No.2 — Complete Member-Pointer Qualifiers

**Expected:** One interpretation. The whole nested-name-specifier belongs to the member-pointer operator.

```C++
int ::qname_fixture::A::b::c::* canonical_member_pointer;
int ::a::b::c::* todo_member_pointer;
```

The qualifier of the first declaration is `::qname_fixture::A::b::c::`; it must not become several nested declarators. The second declaration is the exact regression shape from [`TODO.md`](./TODO.md). Whether those names actually denote namespaces and classes is a later lookup question.

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

## Standard Declaration Ambiguity

C++ contains declaration/expression ambiguities with rules defined in `[dcl.ambig.res]`. These rules should be applied by a focused AST pass rather than scattered through grammar productions.

### Case No.1 — Function Declarations

**Expected:** One interpretation: both are function declarations.

```C++
S v(int(a));
S w(int());
```

### Case No.2 — Object Declarations

**Expected:** One interpretation: each declaration introduces an object.

```C++
S x((int(a)));
S y((int)a);
S z = int(a);
S object_case(B()->C);
```

The extra parentheses or initializer form prevent the first three from being function declarations. The potential parameter declaration in the last line has a trailing return type but does not begin with `auto`, so it remains an object initializer independently of lookup.

### Case No.3 — Lookup-Dependent Trailing Return Type

**Expected:** Preserve ambiguity until `C` is known to be a type.

```C++
S function_case(auto()->C);
```

Leading `auto` enables the function-declaration candidate, but it does not establish that `C` is a type.

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

### Case No.2 — Expression Statements

**Expected:** One interpretation for every line.

```C++
int(expression1)->member = 7;
int(expression2)++;
int(expression3, 5) << value;
S(s)()->M;
```

The final line is an expression even if `M` later resolves to a type, because its outermost declarator candidate does not begin with `auto`.

### Case No.3 — Trailing Return Type Requiring Lookup

**Expected:** Preserve ambiguity until lookup determines whether `M` is a type.

```C++
auto(s)()->M;
```

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

## Type-Id and Expression Ambiguity

The syntax-only rules distinguish some type-id shapes from expressions. Other spellings share the same qualified-name structure and do not benefit from duplicate AST trees.

### Case No.1 — Template Arguments

**Expected:** The first argument is a type-id and the second is an expression.

```C++
X<int()> type_id_template_argument;
X<int(1)> expression_template_argument;
```

### Case No.2 — `sizeof` Operands

```C++
sizeof(int());
sizeof(int(unsigned(a)));
sizeof(int(a));
```

The expected outcomes are:

- `sizeof(int())` and `sizeof(int(unsigned(a)))` select the type-id interpretation. Whether the resulting function type is semantically permitted is outside this pass.
- `sizeof(int(a))` selects the expression interpretation because a named declarator cannot occur in that type-id.

### Case No.3 — Cast or Parenthesized Expression

```C++
(int()) + 1;
(int(unsigned(a))) + 1;
(int(a)) + 1;
```

The first two operands select the type-id interpretation. `int(a)` selects the expression interpretation.

### Case No.4 — Structurally Shared Qualified Names

**Expected:** Retain one lossless spelling tree and defer its semantic category; an ambiguity node containing duplicate qualified-name structures adds no information.

```C++
sizeof(T);
typeid(T);
X<T> value;
```

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

### Case No.2 — Template-Id or Relational Operators

**Expected:** Preserve ambiguity.

```C++
A<B>::C;
Name<a < b>;
```

### Case No.3 — Call, Binary Expression, or Function Type

**Expected:** Preserve all candidates already represented by BuiltIn-Cpp coverage.

```C++
A<B>(C);
```

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

### Case No.3 — Constrained or Non-Type Template Parameter

**Expected:** Preserve ambiguity. Unlike the previous cases, only lookup can determine whether `C` is a concept or a type.

```C++
template<C T>
struct constrained_or_nontype_parameter;
```

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

## Language-Version Ambiguity

### Case No.1 — Multidimensional Subscript or Comma Expression

**Expected:** Preserve the versioned alternatives, or use one lossless index representation that retains the original comma grouping.

```C++
matrix[row, column];
```

In C++23 this can be a two-argument subscript. In an earlier edition it can be a single comma-expression index.

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

### Case No.2 — Split Operator Names

**Expected:** Apply the same normalization to operator-function identifiers.

```C++
struct PracticalOperatorNames
{
    PracticalOperatorNames operator > >(int);
    PracticalOperatorNames operator < <(int);
};
```

### Case No.3 — Longest New-Type-Id Boundary

```C++
new int * i;
```

The standard consumes `*` as the pointer new-declarator and then rejects the stray `i`; it does not parse `(new int) * i`. Accepting the multiplication-shaped overparse is tolerable under the practical policy because the complete standard input is already invalid.

## C++26 Reflection Boundary

Reflection has its own syntax-only longest-operand rule. Its valid boundary pairs and the deliberately excluded invalid form are documented in [`Cpp_Cases_26.md`](./Cpp_Cases_26.md) under **Longest Reflect-Expression Boundary**.
