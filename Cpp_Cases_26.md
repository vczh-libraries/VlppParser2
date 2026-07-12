# C++ 26 Missing Features

This file contains phase-7 syntax and AST gaps in the current C++26 working draft, N5046. Token formation for `^^`, `[:`, `:]`, `contract_assert`, raw strings, and `#embed` is covered separately in `Cpp_Cases_Tokenizer.md`.

C++26 adds several genuinely new grammar families: pack indexing, structured-binding extensions, variadic friends, new template-template parameter kinds, contracts, reflection and splicing, annotations, consteval blocks, and expansion statements. None has a corresponding BuiltIn-Cpp AST today. Semantic evaluation of reflection, constraints, or contracts is outside this audit; the parser only needs to retain their written structure and any syntactic ambiguity.

## Pack Indexing

Both the expression and type computed-specifier forms are missing. The index is a constant-expression enclosed by brackets after an ellipsis.

### Case No.1

```C++
args...[0]
```

### Case No.2

```C++
args...[sizeof...(args) - 1]
```

### Case No.3

```C++
template<class... Ts>
using First = Ts...[0];

template<class... Ts>
using Last = Ts...[sizeof...(Ts) - 1];

template<class... Ts>
void consume(Ts...[1] value);
```

## Attributes on Individual Structured Bindings

An attribute-specifier sequence can follow each `sb-identifier`, independently of attributes on the whole structured-binding declaration.

### Case No.1

```C++
auto [first [[maybe_unused]], second] = value;
```

### Case No.2

```C++
auto [first, second [[maybe_unused]]] = value;
```

## Structured-Binding Packs

One structured binding can carry the ellipsis and introduce a pack. It can appear at any position in the identifier list, including as the only element. A declaration containing a structured-binding pack must declare a templated entity, so every case is placed in a function template with a dependent initializer.

### Case No.1

```C++
template<typename T>
void bindAll(T value)
{
    auto [...all] = value;
}
```

### Case No.2

```C++
template<typename T>
void bindPrefix(T value)
{
    auto [...prefix, last] = value;
}
```

### Case No.3

```C++
template<typename T>
void bindSuffix(T value)
{
    auto [first, ...suffix] = value;
}
```

### Case No.4

```C++
template<typename T>
void bindMiddle(T value)
{
    auto [first, ...middle, last] = value;
}
```

## Structured-Binding Declarations as Conditions

`condition-declaration` gains a structured-binding branch. Coverage should exercise every statement consumer. Whether the decision variable is valid in each example is semantic and intentionally outside the parser.

### Case No.1

```C++
void f()
{
    if (auto [x, y] = source())
    {
    }

    while (auto [x, y] = source())
    {
    }
}
```

### Case No.2

```C++
void f()
{
    for (; auto [x, y] = source();)
    {
    }

    switch (auto [x, y] = source())
    {
    }
}
```

## Constant-Initialization Specifiers on Structured Bindings

`constexpr` and `constinit` are now permitted in the structured binding's decl-specifier sequence. `constexpr` should also compose with the new binding-pack form; `constinit` composes with its required static or thread storage duration.

### Case No.1

```C++
constexpr auto [first, second] = value;
```

### Case No.2

```C++
template<typename T>
void bindConstexprPack()
{
    constexpr auto [first, ...rest] = T::value;
}
```

### Case No.3

```C++
static constinit auto [initializedFirst, initializedSecond] = value;
```

## Deleted-Function Reasons

A deleted-function body can contain one unevaluated string explaining the deletion.

### Case No.1

```C++
void old_api() = delete("use new_api instead");
```

### Case No.2

```C++
struct Value
{
    Value(const Value&) = delete("Value is move-only");
};
```

## Variadic Friend Type Declarations

Friend type declarations now contain a comma-separated list, and each type specifier may be pack-expanded. Coverage must include simple, elaborated, and dependent `typename` forms. `friend class Ts...` itself is not a valid way to expand arbitrary type parameters; use an elaborated template-id for that branch.

### Case No.1

```C++
class A;
class B;

template<class... Ts>
struct Friends
{
    friend Ts...;
    friend A, B, Ts...;
};
```

### Case No.2

```C++
template<class>
class C;

template<class... Ts>
struct TemplateFriends
{
    friend class C<Ts>...;
    friend typename Ts::Nested...;
};
```

## Variable and Concept Template-Template Parameters

Template-template parameters gain `auto` and `concept` parameter kinds. Each supports an ordinary parameter, a pack, and a default template name.

### Case No.1

```C++
template<class>
inline constexpr bool PredicateValue = true;

template<template<class> auto Value>
struct VariableParameter;

template<template<class> auto... Values>
struct VariableParameterPack;

template<template<class> auto Value = PredicateValue>
struct DefaultVariableParameter;

template<template<class> auto>
struct UnnamedVariableParameter;

template<template<class> auto...>
struct UnnamedVariableParameterPack;
```

### Case No.2

```C++
template<class>
concept PredicateConcept = true;

template<template<class> concept Constraint>
struct ConceptParameter;

template<template<class> concept... Constraints>
struct ConceptParameterPack;

template<template<class> concept Constraint = PredicateConcept>
struct DefaultConceptParameter;

template<template<class> concept>
struct UnnamedConceptParameter;

template<template<class> concept...>
struct UnnamedConceptParameterPack;
```

## Contract Assertion Statements

`contract_assert` introduces an assertion statement with an optional attribute-specifier sequence and a conditional-expression predicate. The unattributed form is an **AST fidelity gap** when `contract_assert` is still tokenized as `ID`: a block-statement entry can accept it as an ordinary call expression. The attributed form is a text-acceptance gap, and both forms need a dedicated assertion node for indexing.

### Case No.1

```C++
contract_assert(value > 0);
```

### Case No.2

```C++
contract_assert [[ ]] (value > 0);
```

## Function Preconditions and Postconditions

Function-contract specifiers follow the complete declarator and optional requires-clause. Coverage must include multiple specifiers, attributes, a postcondition result-name introducer, attributes on that result name, declarations/definitions, and lambdas.

### Case No.1

```C++
int checked(int value)
    pre(value > 0)
    post(result: result > 0);
```

### Case No.2

```C++
int attributed(int value)
    pre [[ ]] (value != 0)
    post [[ ]] (result [[maybe_unused]]: result != 0)
{
    contract_assert [[ ]] (value > 0);
    return value;
}
```

### Case No.3

```C++
template<class T>
int constrained(T value)
    requires C<T>
    pre(valid(value))
    post(result: valid(result));
```

### Case No.4

```C++
[](int value)
    pre(value > 0)
    post(result: result > 0)
{
    return value;
}
```

## Reflect Expressions

The `^^` prefix can reflect the global namespace, a reflection-name, a type-id, or an id-expression. The parser must retain the operand category candidates because name lookup can distinguish them later.

### Case No.1

```C++
^^::
```

### Case No.2

```C++
^^entity
```

### Case No.3

```C++
^^namespace_name::entity
```

### Case No.4

```C++
^^T::template member
```

### Case No.5

```C++
^^T::template member<int>
```

### Case No.6

```C++
^^int
```

### Case No.7

```C++
^^int*
```

## Longest Reflect-Expression Boundary

The standard requires `^^` to consume the longest token sequence that can syntactically form a reflect-expression. This is a parser priority/post-recognition filter, not symbol resolution. Parentheses explicitly terminate the operand. All cases below are well-formed boundary spellings; the deliberately excluded `^^X < value` form is ill-formed because an unparenthesized reflection representing a template cannot be followed by `<`.

### Case No.1

```C++
auto reflectedReferenceType = ^^int&&;
```

### Case No.2

```C++
r == (^^int) && true
```

### Case No.3

```C++
(^^X) < value
```

### Case No.4

```C++
^^X<true> < value
```

## Splice Expressions and Splice Specializations

A splice specifier is a primary expression. The leading `template` variants designate a function template or a specialization and need separate AST forms.

### Case No.1

```C++
[:reflection:]
```

### Case No.2

```C++
template [:template_reflection:]
```

### Case No.3

```C++
template [:template_reflection:]<>
```

### Case No.4

```C++
template [:template_reflection:]<int>
```

## Spliced Types

A splice can be a type in a type-only context, can be forced by `typename`, can carry template arguments, and can appear as a base type.

### Case No.1

```C++
using ImplicitType = [:type_reflection:];
using Reflected = typename [:type_reflection:];
```

### Case No.2

```C++
using EmptySpecialization = typename [:template_reflection:]<>;
using Specialization = typename [:template_reflection:]<int>;
```

### Case No.3

```C++
struct Derived : [:base_reflection:]
{
};
```

## Computed Destructor Names

A destructor unqualified-id can use a computed type specifier. Member access through both `.` and `->` must therefore admit a splice-derived destructor name.

### Case No.1

```C++
object.~[:type_reflection:]()
```

### Case No.2

```C++
pointer->~[:type_reflection:]()
```

## Spliced Scopes and Members

Splice specifiers and splice specializations can form nested-name-specifiers. Splice expressions also appear directly after `.` and `->` member access.

### Case No.1

```C++
[:scope_reflection:]::member
```

### Case No.2

```C++
template [:template_reflection:]<int>::member
```

### Case No.3

```C++
object.[:member_reflection:]
```

### Case No.4

```C++
pointer->template [:member_template_reflection:]<int>
```

## Splices in Type Requirements

Requires-expressions add dedicated type-requirement branches for splice specifiers and splice specializations.

### Case No.1

```C++
template<class R>
concept ReflectedType = requires
{
    typename [:R:];
    typename [:R:]<int>;
};
```

## Namespace and Enum Splices

Namespace aliases, using-directives, and using-enum declarations each gain a splice operand.

### Case No.1

```C++
namespace alias = [:namespace_reflection:];
```

### Case No.2

```C++
using namespace [:namespace_reflection:];
```

### Case No.3

```C++
using enum [:enum_reflection:];
```

## Reflection Annotations

An annotation attribute-specifier contains one or more `= constant-expression` entries, each optionally pack-expanded. This is a new alternative to an ordinary attribute list and can occur at every attribute attachment point, including base specifiers.

### Case No.1

```C++
[[=1]] void first();
```

### Case No.2

```C++
[[=2, =3, =2]] void second();
void second [[=4, =2]] ();
```

### Case No.3

```C++
template<class... Ts>
struct Annotated
{
    int member [[=(^^Ts) ...]];
};
```

### Case No.4

```C++
struct Base
{
};

struct Derived : [[=1]] Base
{
};
```

## Consteval Block Declarations

A `consteval` followed by a compound statement is a declaration. It is permitted at namespace, block, and class scope and needs its own AST rather than being confused with a function specifier.

### Case No.1

```C++
consteval
{
}
```

### Case No.2

```C++
void f()
{
    consteval
    {
    }
}
```

### Case No.3

```C++
struct S
{
    consteval
    {
    }
};
```

## Expansion Statements

`template for` is a new statement whose body must be a compound statement. Its initializer can be an expression or a dedicated brace list, and it reuses init-statements and ordinary/structured range declarations.

### Case No.1

```C++
consteval void enumerate(auto const& range)
{
    template for (auto const& value : range)
    {
    }
}
```

### Case No.2

```C++
consteval void enumerate()
{
    template for (int value : {1, 2, 3})
    {
    }

    template for (int value : {1, 2, 3,})
    {
    }

    template for (int value : {})
    {
    }
}
```

### Case No.3

```C++
consteval void enumerate(auto const& range)
{
    template for (int seed = 0; auto const& value : range)
    {
    }

    template for (auto [key, value] : range)
    {
    }
}
```

## Generalized Asm Payloads

C++26 changes the conditionally-supported standard asm declaration from one string-literal to an arbitrary nonempty balanced-token sequence. The payload meaning remains implementation-defined, but balanced parentheses, brackets, braces, and splice delimiters are standard grammar input. With `asm` still tokenized as `ID`, a call-shaped spelling can be accepted as an ordinary expression statement inside a block, which is an **AST fidelity gap**; the same spelling at namespace scope and non-expression payloads remain text-acceptance gaps.

### Case No.1

```C++
asm("instruction", operand(value));
```

### Case No.2

```C++
asm("instruction" (nested) [tokens] {more_tokens});
```

### Case No.3

```C++
asm([: tokens :]);
```

## Indeterminate Attribute

`[[indeterminate]]` is a new standard attribute for function parameters and uninitialized automatic block variables. It uses the generic attribute grammar but deserves version-specific coverage.

### Case No.1

```C++
void f([[indeterminate]] int parameter)
{
    int local [[indeterminate]];
}
```

## Expanded Maybe-Unused Subjects

C++26 permits `[[maybe_unused]]` on identifier labels and contract result bindings. The result-binding attachment is covered in the contract cases; the trailing label case exercises the new label subject together with C++23 trailing-label syntax.

### Case No.1

```C++
void f()
{
    goto done;

[[maybe_unused]] done:
}
```

## Already Covered or Practically Accepted

- A user-generated `static_assert` message is already accepted because `_StaticAssertDecl` permits any `_Expr_NoComma` as its message.
- The name-independent placeholder `_` is already an ordinary `ID`; its special declaration rules are semantic.
- The C++26 treatment of a comma-less trailing ellipsis does not add a new accepted spelling. Historical cases remain in `Cpp_Cases_17.md`, and the type-dependent interpretation belongs in `Cpp_Cases_Improvement.cpp`.
- Unevaluated-string changes mostly restrict already broad string acceptance, which does not require work under the practical policy.
- Trivial unions, constexpr exception handling, constexpr virtual inheritance, observable checkpoints, and most other constexpr changes are semantic.
- `#embed` and reflection punctuator formation belong in the tokenizer/preprocessor layer.

## Standards References

- [N5046 — current C++26 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/n5046.pdf)
- [P2893R3 — variadic friend declarations](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2893r3.html)
- [P2841R7 — concept and variable template-template parameters](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2841r7.pdf)
- [P2996R13 — static reflection](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r13.html)
- [P2361R6 — unevaluated strings and generalized asm payloads](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2361r6.pdf)
