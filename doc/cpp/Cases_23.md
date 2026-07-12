# C++ 23 Missing Features

## Feature and Case Index

- Explicit Object Parameters
  - Case No.1
  - Case No.2
- If-Consteval Statements
  - Case No.1
  - Case No.2
- Omitted Lambda Parentheses with C++23 Specifiers
  - Case No.1
  - Case No.2
- Static Lambdas
  - Case No.1
  - Case No.2
  - Case No.3
- Multidimensional Subscript Expressions
  - Case No.1
  - Case No.2
  - Case No.3
  - Case No.4
- Attributes Immediately After a Lambda Introducer
  - Case No.1
  - Case No.2
  - Case No.3
- Alias Declarations in Init-Statements
  - Case No.1
  - Case No.2
- Labels at the End of Compound Statements
  - Case No.1
  - Case No.2
- Attributes on Concept Definitions
  - Case No.1
- Assume Attribute
  - Case No.1
  - Case No.2

This file contains phase-7 syntax and AST gaps introduced by C++23. Token and preprocessing changes, including literal suffixes, escape forms, and new directives, are listed in [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md).

The current grammar already accepts several famous C++23 spellings, including `auto(expression)`, `auto{expression}`, static `operator()`/`operator[]` declarations, and `object[x, y]` through its older comma-expression AST. The cases below focus on rejected spellings and on AST shapes that cannot distinguish the C++23 construct.

Apply the [C++ Syntax Implementation Philosophy](Philosophy.md) throughout: this is a union-of-versions indexer grammar, so a shared orthogonal rule and a bounded invalid superset are preferred to edition-specific branches, semantic validation, or otherwise unnecessary ambiguity.

Implementation categories used below:

- **Implementation suggestion — Additive:** introduce a wholly absent syntax family in the AST and grammar box that owns it.
- **Implementation suggestion — Structural:** generalize an existing shared representation so all affected contexts compose from one orthogonal design.

Bounded practical over-acceptance is preferred when a small invalid superset substantially improves orthogonality and cannot misinterpret valid, compiler-verified input.

## Explicit Object Parameters

Function parameters have no optional leading `this`. Coverage must include concrete and dependent explicit object types and the lambda form.

### Case No.1

```C++
struct S
{
    void f(this S& self);

    template<class Self>
    void g(this Self&& self, int value);
};
```

### Case No.2

```C++
[](this auto&& self, int n) -> int
{
    return n <= 1 ? 1 : n * self(n - 1);
}
```

**Implementation suggestion — Structural:** Introduce a dedicated function-parameter declaration shape in `Syntax/Ast/DeclsFuncVar.txt` with an optional explicit-object (`this`) marker, instead of representing every parameter only as a generic `VariablesDeclaration`. Update `Syntax/Syntax/DeclarationVariable.txt::_FunctionParameter` and `DeclaratorComponents.txt::_DeclaratorFunctionParameters` to consume it so member functions and lambdas share one parameter model. Let the shared parameter rule accept the marker wherever a parameter is accepted; deliberately leaving first-position and member-function restrictions to compiler validation is a bounded invalid superset and avoids a context matrix.

## If-Consteval Statements

`if consteval` and `if !consteval` do not use parentheses and have their own selection-statement alternatives. Both optional-`else` forms are missing.

### Case No.1

```C++
constexpr int f()
{
    if consteval
    {
        return 1;
    }

    if consteval
    {
        return 2;
    }
    else
    {
        return 3;
    }
}
```

### Case No.2

```C++
constexpr int g()
{
    if !consteval
    {
        return 1;
    }

    if !consteval
    {
        return 2;
    }
    else
    {
        return 3;
    }
}
```

**Implementation suggestion — Additive:** Add an `IfConstevalStat` node, including the negation flag and optional else branch, to `Syntax/Ast/Statements.txt`, and add its alternatives beside `_IfStat` in `Syntax/Syntax/Statements.txt`. It is a separate selection form because it has no parenthesized condition and its selected substatements are compound statements; the existing dangling-else priority should still be reused.

## Omitted Lambda Parentheses with C++23 Specifiers

BuiltIn-Cpp already accepts several no-parameter-list forms such as `[] mutable {}`, `[] constexpr {}`, `[] noexcept {}`, and `[] -> int {}`. The C++23 omission rule also exposes specifiers and clauses that are otherwise missing.

### Case No.1

```C++
[] consteval {}
```

### Case No.2

```C++
[] static {}
```

**Implementation suggestion — Structural:** Keep `Syntax/Syntax/DeclaratorComponents.txt::_DeclaratorFunctionPartOptionalParameters` as the single optional-parentheses path already composed by `Syntax/Syntax/Expressions.txt::_LambdaExpr`, and add `consteval` and `static` to its shared function-keyword input. No AST flag is needed merely to distinguish `[]()` from `[]` for indexing. Reusing the keyword path deliberately accepts these words in some invalid ordinary-function suffix positions, but adds no ambiguity and is the smaller orthogonal design.

## Static Lambdas

`static` is accepted before ordinary function declarations, but it is absent from `_FunctionKeyword` and therefore from lambda specifier sequences.

### Case No.1

```C++
[]() static {}
```

### Case No.2

```C++
[] static {}
```

### Case No.3

```C++
[]<class T>(T value) static
{
    return value;
}
```

**Implementation suggestion — Structural:** Add `static` to the existing `Syntax/Syntax/DeclaratorComponents.txt::_FunctionKeyword` mapping and let both `_DeclaratorFunctionPart` forms continue to compose the same suffix model used by lambdas. This intentionally also admits `static` in invalid ordinary-function suffix positions; compiler-verified input removes that bounded superset, while a lambda-only specifier hierarchy would duplicate the same parameter, exception, and trailing-return topology.

## Multidimensional Subscript Expressions

C++23 changes the subscript operand to an optional initializer-list. `matrix[row, column]` is textually accepted today as one `IndexExpr` whose index is a comma `BinaryExpr`; that is an **AST fidelity gap** because it does not represent multiple arguments. Zero arguments and pack-expanded arguments are rejected outright.

Before C++23 the same token sequence could denote one comma-expression index. The indexer should keep accepting that historical spelling, but the philosophy says to prefer the newer interpretation when identical tokens have two edition-specific meanings. It does not need a second parse candidate or a language-version post-pass; an explicitly parenthesized comma expression such as `matrix[(row, column)]` remains one argument.

### Case No.1

```C++
matrix[row, column]
```

### Case No.2

```C++
matrix[]
```

### Case No.3

```C++
matrix[indices...]
```

### Case No.4

```C++
matrix[row, indices...]
```

**Implementation suggestion — Structural:** Change `Syntax/Ast/Expressions.txt::IndexExpr` from one `index` expression to an optional initializer-clause argument list, and make the postfix-index rule in `Syntax/Syntax/Expressions.txt` consume the same initializer-list component used by calls and braced initialization. Parse an unparenthesized comma as the C++23 argument separator, while continuing to accept the historical source spelling with that newer AST; do not introduce an edition-only ambiguity alternative.

## Attributes Immediately After a Lambda Introducer

C++23 adds an attribute-specifier sequence between the lambda introducer or explicit template/requires prefix and the lambda declarator. This is distinct from the older attribute position after a parameter list.

### Case No.1

```C++
[] [[nodiscard]] () -> int
{
    return 0;
}
```

### Case No.2

```C++
[] [[nodiscard]]
{
    return 0;
}
```

### Case No.3

```C++
[]<class T> requires C<T> [[nodiscard]] (T value)
{
    return value;
}
```

**Implementation suggestion — Structural:** Reuse the canonical `AttributeSpecifier` family from `Syntax/Ast/Attributes.txt` and `Syntax/Syntax/Attributes.txt`, but give `Syntax/Ast/Expressions.txt::LambdaExpr` distinct front-attribute and declarator-attribute fields. Update `Syntax/Syntax/Expressions.txt::_LambdaExpr` at the two actual attachment points so source position and meaning are retained instead of collecting all lambda attributes into an undifferentiated token list.

## Alias Declarations in Init-Statements

C++23 adds alias-declaration to `init-statement`. Every consumer should be covered: `if`, `switch`, classic `for`, and range-`for`.

### Case No.1

```C++
void f(Range values)
{
    if (using T = int; true)
    {
    }

    switch (using T = int; 0)
    {
    }
}
```

### Case No.2

```C++
void f(Range values)
{
    for (using T = int; false;)
    {
    }

    for (using T = int; T value : values)
    {
    }
}
```

**Implementation suggestion — Structural:** Make the canonical `InitStatement` family in `Syntax/Ast/Statements.txt` and `Syntax/Syntax/Statements.txt` include an alias-declaration alternative that reuses the declaration core of `Syntax/Syntax/DeclarationOthers.txt::_UsingTypeDecl`. Then have `if`, `switch`, classic `for`, and range-`for` consume that same family; four local `using` alternatives would duplicate delimiter ownership and precedence.

## Labels at the End of Compound Statements

`LabelStat`, `CaseStat`, and `DefaultStat` all require a following statement. C++23 permits a trailing label sequence immediately before `}`.

### Case No.1

```C++
void f(bool condition)
{
    if (condition)
    {
        goto done;
    }

done:
}
```

### Case No.2

```C++
void g(int value)
{
    switch (value)
    {
    case 0:
    case 1:
    default:
    }
}
```

**Implementation suggestion — Structural:** Make `LabelStat.stat`, `CaseStat.stat`, and `DefaultStat.stat` optional in `Syntax/Ast/Statements.txt`, and update the three existing alternatives in `Syntax/Syntax/Statements.txt` to use the preferred optional form `+[_Stat:stat]`. The preference keeps a following statement attached when one exists, recursion already represents stacked labels, and the nullable tail admits the C++23 block-ending form without a parallel label hierarchy. This also accepts a missing substatement in a few non-block-final positions, which is a bounded invalid superset left to compiler validation.

## Attributes on Concept Definitions

CWG 2428 added the attribute position after a concept name. It was adopted during the C++23 cycle. This case depends on both the missing concept AST and the missing general attribute grammar.

### Case No.1

```C++
template<class T>
concept C [[deprecated]] = true;
```

**Implementation suggestion — Additive:** In the canonical concept/constraint boxes (`Syntax/Ast/Constraints.txt` and `Syntax/Syntax/Constraints.txt`), give `ConceptDeclaration` an attribute sequence immediately after its name and consume the shared `AttributeSpecifier` family from the attribute boxes. Keeping the attachment on the concept node avoids treating `deprecated` as part of either the name or constraint expression.

## Assume Attribute

`[[assume]]` needs no special production once generic balanced-token attributes exist, but it is a currently rejected standard C++23 case and should have a version-specific regression.

### Case No.1

```C++
[[assume(value >= 0)]];
```

### Case No.2

```C++
void f(int value)
{
    [[assume(value >= 0)]];
}
```

**Implementation suggestion — Additive:** Implement `assume` through the generic balanced-token nodes in `Syntax/Ast/Attributes.txt` and `Syntax/Syntax/Attributes.txt`, and add the shared attribute attachment to the empty-statement/attribute-declaration hosts in `Syntax/Ast/Statements.txt`, `Syntax/Syntax/Statements.txt`, and the declaration router. No `assume`-specific expression production is needed: its argument remains attribute-token data for indexing.

## Already Covered or Practically Accepted

- `auto(expression)` and `auto{expression}` are accepted as type-shaped calls; a dedicated edition-specific AST would not add useful indexing information.
- Static `operator()` and static `operator[]` declarations use existing declaration keywords and operator identifiers.
- `object[x, y]` is accepted, subject to the AST fidelity issue above.
- Simpler implicit move, constexpr relaxations, inherited-constructor deduction, and range-for lifetime extension are semantic changes.
- Literal suffixes, character/escape changes, floating suffixes, and preprocessing directives belong in the tokenizer document.

## Standards References

- [N4950 — final C++23 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf)
- [P2128R6 — multidimensional subscripting](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2128r6.pdf)
- [P2324R2 — labels at the end of compound statements](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2324r2.pdf)
- [P2360R0 — alias declarations in init-statements](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2360r0.html)
- [P2173R1 — attributes on lambda expressions](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2173r1.pdf)
