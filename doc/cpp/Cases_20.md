# C++ 20 Missing Features

This file contains phase-7 syntax and AST gaps introduced by C++20. It assumes the input has already passed through a C++ scanner and preprocessor; missing keyword tokens, header-name recognition, and preprocessing are recorded in `Cpp_Cases_Tokenizer.md`.

The largest missing families are modules, constraints/requires syntax, coroutines, and new declaration specifiers. Several C++20 spellings happen to fit an older generic BuiltIn-Cpp production. Those cases are marked as **AST fidelity gaps** because accepting text as the wrong kind of declaration is not enough for a code indexer.

## Modules and Export Declarations

There are no module-unit, module declaration, import, partition, fragment, or C++20 export productions. Each case is a separate translation unit because module declarations have ordering constraints. Header-unit cases additionally require context-sensitive tokenizer support.

### Case No.1

```C++
export module Primary;
export int function();
```

### Case No.2

```C++
module Implementation;
```

### Case No.3

```C++
export module Library.Core:Interface;
```

### Case No.4

```C++
module Library.Core:Implementation;
```

### Case No.5

```C++
module;
export module Library;
```

### Case No.6

```C++
export module Library;
module :private;
int implementationDetail();
```

### Case No.7

```C++
export module Consumer;
import Dependency.Core;
import :Partition;
export import PublicDependency;
```

### Case No.8

```C++
export module HeaderConsumer;
import <standard_header>;
import "local_header.hpp";
```

### Case No.9

```C++
export module Grouped;
export
{
    int first();
    int second();
}
```

## Concepts, Constraints, and Constrained Placeholders

There is no concept-definition, type-constraint, or requires-clause AST. `template<C T>` and `template<C... Ts>` can already be consumed as non-type parameters whose type is the qualified name `C`; when `C` is a concept, that is the wrong AST interpretation. A symbol-blind parser should retain both parameter candidates until lookup.

### Case No.1

```C++
template<typename T>
concept Any = true;

template<Any T>
struct Box
{
};

template<Any... Types>
struct Pack
{
};

template<Any>
struct Unnamed;

template<Any...>
struct UnnamedPack;
```

### Case No.2

```C++
template<typename T>
    requires Any<T>
void prefixConstraint(T);

template<typename T>
void trailingConstraint(T) requires Any<T>;
```

### Case No.3

```C++
void abbreviated(Any auto value);

Any auto constrainedValue = 0;

Any decltype(auto) identity(int& value)
{
    return (value);
}
```

## Requires Expressions

All four requirement forms are missing: simple, type, compound, and nested requirements. Both the parameterized and parameterless requires-expression forms need entry coverage.

### Case No.1

```C++
template<typename T>
concept Complete = requires(T value)
{
    value + value;
    typename T::value_type;
    { value.function() };
    { value.function() } noexcept;
    { value.function() } -> Any;
    { value.function() } noexcept -> Any;
    requires Any<T>;
};
```

### Case No.2

```C++
requires
{
    expression;
}
```

### Case No.3

```C++
requires()
{
    expression;
}
```

## Lambda Constraints

Explicit lambda template parameter lists are already accepted. The requires-clause after that list, constrained lambda template parameters, and the trailing lambda requires-clause are missing.

### Case No.1

```C++
template<typename T>
concept Any = true;

auto lambda =
    []<Any T> requires Any<T>
    (T value) requires Any<decltype(value)>
    {
        return value;
    };
```

### Case No.2

The requires-clause after an explicit template parameter list does not require a lambda parameter list, even before C++23's general relaxation for omitted lambda parentheses.

```C++
template<typename T>
concept Any = true;

auto noFunctionParameters = []<typename T> requires Any<T>
{
};
```

## Coroutines

There are no await-expression, yield-expression, coroutine-return-statement, or `operator co_await` productions. The operand categories and braced alternatives should remain distinct in the AST.

### Case No.1

```C++
co_await task
```

### Case No.2

```C++
co_yield value
```

### Case No.3

```C++
co_yield {first, second}
```

### Case No.4

```C++
co_return;
```

### Case No.5

```C++
co_return value;
```

### Case No.6

```C++
co_return {first, second};
```

### Case No.7

```C++
struct Awaitable
{
    int operator co_await();
};
```

## Designated Initializers

Braced initializer lists have no `. identifier` designator branch. Coverage should include equals and braced initialization and the optional trailing comma.

### Case No.1

```C++
Point{.x = 1, .y = 2}
```

### Case No.2

```C++
Point{.x{1}, .y = {2},}
```

## Array-Bound Deduction in New Expressions

C++20 permits an omitted first array bound when an initializer supplies the size. The current new-expression rule requires an expression in every pair of brackets. The rest of the full new-type-id gap is documented in `Cpp_Cases_17.md`.

### Case No.1

```C++
new Type[]{first, second, third}
```

## Range-For Init-Statements

C++20 range-for statements can begin with the same init-statement family used by selection statements. The current range-for branch begins directly with its range declaration.

### Case No.1

```C++
void process();

void function()
{
    int values[2]{};

    for (process(); int value : values)
    {
    }
}
```

### Case No.2

```C++
void function()
{
    int values[2]{};

    for (; int value : values)
    {
    }
}
```

### Case No.3

```C++
void function()
{
    int values[2]{};

    for (int count = 0; int value : values)
    {
        ++count;
    }
}
```

## Static and Thread-Local Structured Bindings

C++20 permits `static` and `thread_local` in a structured-binding declaration. This is a syntax extension to the C++17 declaration rather than merely a storage-duration rule, so both namespace and block scopes should be represented.

### Case No.1

```C++
struct Pair
{
    int first;
    int second;
};

Pair pair{};
static auto [staticFirst, staticSecond] = pair;
thread_local auto [threadFirst, threadSecond] = pair;
```

### Case No.2

```C++
struct Pair
{
    int first;
    int second;
};

void function(Pair pair)
{
    static auto [staticFirst, staticSecond] = pair;
    thread_local auto [threadFirst, threadSecond] = pair;
}
```

## Consteval Declarations and Lambdas

`consteval` is absent from declaration and lambda specifiers.

### Case No.1

```C++
consteval int immediate(int value)
{
    return value;
}

struct Value
{
    consteval Value(int);
};
```

### Case No.2

```C++
[]() consteval
{
    return 1;
}
```

## Constinit Declarations

`constinit` is absent from declaration specifiers and must compose with `thread_local` and static local declarations.

### Case No.1

```C++
constinit int globalValue = 0;
thread_local constinit int threadValue = 0;

void function()
{
    static constinit int localValue = 0;
}
```

## Conditional Explicit Specifiers

`explicit` is currently a bare declaration keyword. The parenthesized constant-expression form is missing on constructors, conversion functions, and deduction guides.

### Case No.1

```C++
template<typename T>
struct Wrapper
{
    explicit(sizeof(T) > 1) Wrapper(T);
    explicit(sizeof(T) == 1) operator bool() const;
};
```

### Case No.2

```C++
template<typename T>
explicit(sizeof(T) > 1) Wrapper(T) -> Wrapper<T>;
```

## Nested Inline Namespace Components

C++17 nested namespace definitions are already accepted, but C++20 permits `inline` on individual nested components. `_NsDecl` accepts only identifiers separated by `::`.

### Case No.1

```C++
namespace Library::inline Version1::Detail
{
}

namespace Library::Version2::inline Detail
{
}
```

## Three-Way Comparison Operator Declarations

`<=>` expressions are implemented, but an `operator<=>` identifier is not. The lexer emits one `COMPARE` token while `_OperatorIdentifier` asks for three terminals, `"<" "=" ">"`.

### Case No.1

```C++
struct Value
{
    int member;

    int operator<=>(const Value&) const;
};
```

### Case No.2

```C++
struct Defaulted
{
    int member;

    auto operator<=>(const Defaulted&) const = default;
};
```

## C++20 Standard Attribute Uses

Generic attribute syntax is an earlier missing feature, but these C++20-standardized uses exercise new valid programs and attachment positions. They should become ordinary attribute nodes rather than feature-specific grammar rules.

### Case No.1

```C++
struct Empty
{
};

struct Data
{
    [[no_unique_address]] Empty marker;
    int value;
};
```

### Case No.2

```C++
[[nodiscard("the result must be checked")]]
int calculate();
```

### Case No.3

```C++
int choose(bool condition)
{
    if (condition)
        [[likely]] return 1;
    else
        [[unlikely]] return 0;
}
```

## Using-Enum AST Classification

This is an **AST fidelity gap** rather than a text-acceptance gap. `using enum Color;` can be consumed as `UsingValueDeclaration` because the qualified-name grammar has an enum-shaped identifier. The explicit `enum` keyword makes reclassification possible without lookup, but a dedicated node is preferable for indexing.

### Case No.1

```C++
enum class Color
{
    red,
    green,
};

using enum Color;
```

### Case No.2

```C++
enum class Color
{
    red,
    green,
};

using enum ::Color;
```

## Already Covered or Practically Accepted

The following C++20 features do not need missing-case entries:

- `char8_t` and `<=>` expressions already have grammar support.
- Explicit lambda template parameter lists and lambda init-capture packs are accepted.
- `[=, this]`, bit-field default initializers, and lambdas in unevaluated operands fit existing productions.
- Parenthesized aggregate initialization is already accepted as a type-shaped call; aggregate validity is semantic.
- Class-type non-type template parameter spellings use the existing type/expression template parameter model.
- Plain defaulted `operator==` declarations are accepted.

## Standards References

- [N4861 — C++20 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf)
- [P2131R0 — changes between C++17 and C++20](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2131r0.html)
- [P1091R3 — extending structured bindings](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1091r3.html)
