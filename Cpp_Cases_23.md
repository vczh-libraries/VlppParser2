# C++ 23 Missing Features

This file contains phase-7 syntax and AST gaps introduced by C++23. Token and preprocessing changes, including literal suffixes, escape forms, and new directives, are listed in `Cpp_Cases_Tokenizer.md`.

The current grammar already accepts several famous C++23 spellings, including `auto(expression)`, `auto{expression}`, static `operator()`/`operator[]` declarations, and `object[x, y]` through its older comma-expression AST. The cases below focus on rejected spellings and on AST shapes that cannot distinguish the C++23 construct.

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

## Multidimensional Subscript Expressions

C++23 changes the subscript operand to an optional initializer-list. `matrix[row, column]` is textually accepted today as one `IndexExpr` whose index is a comma `BinaryExpr`; that is an **AST fidelity gap** because it does not represent multiple arguments. Zero arguments and pack-expanded arguments are rejected outright.

For a union-of-versions indexer, the comma AST cannot simply be discarded: before C++23 the same token sequence can denote one comma-expression index. Preserve both interpretations or use an index-argument representation that retains the original comma grouping and language-version choice.

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

## Attributes on Concept Definitions

CWG 2428 added the attribute position after a concept name. It was adopted during the C++23 cycle. This case depends on both the missing concept AST and the missing general attribute grammar.

### Case No.1

```C++
template<class T>
concept C [[deprecated]] = true;
```

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

## Already Covered or Practically Accepted

- `auto(expression)` and `auto{expression}` are accepted as type-shaped calls.
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
