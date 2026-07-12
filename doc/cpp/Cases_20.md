# C++ 20 Missing Features

This file contains phase-7 syntax and AST gaps introduced by C++20. It assumes the input has already passed through a C++ scanner and preprocessor; missing keyword tokens, header-name recognition, and preprocessing are recorded in [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md).

The largest missing families are modules, constraints/requires syntax, coroutines, and new declaration specifiers. Several C++20 spellings happen to fit an older generic BuiltIn-Cpp production. Those cases are marked as **AST fidelity gaps** because accepting text as the wrong kind of declaration is not enough for a code indexer.

Implementation categories used below:

- **Implementation suggestion — Additive:** introduce a wholly absent syntax family in the AST and syntax box that owns that family, then connect it to the existing public routers.
- **Implementation suggestion — Structural:** generalize an existing shared representation and route all affected consumers through the redesigned rule instead of adding a context-specific production.

Bounded practical over-acceptance is preferred when a small invalid superset substantially improves orthogonality and cannot misinterpret valid, compiler-verified input.

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

**Implementation suggestion — Additive:** Create dedicated `Syntax/Ast/DeclsModule.txt` and `Syntax/Syntax/DeclarationModules.txt` boxes for module declarations, imports, exports, partitions, and fragments. Route declaration-shaped forms through `Syntax/Syntax/Declarations.txt`, and generalize only `Syntax/Syntax/API.txt::_File` to own the ordered module-unit envelope.

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

**Implementation suggestion — Structural:** Introduce canonical constraint AST/rule boxes, such as `Syntax/Ast/Constraints.txt` and `Syntax/Syntax/Constraints.txt`, then make `Syntax/Syntax/Generic.txt::_GenericParameter`, the placeholder-type family in `Syntax/Syntax/Types.txt`, and function/declaration consumers in `DeclaratorComponents.txt` and `Declarations.txt` reference those nodes. This preserves the existing type-versus-value ambiguity while preventing each constrained context from inventing its own concept spelling.

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

**Implementation suggestion — Additive:** Add requires-expression and the four requirement node kinds to `Syntax/Ast/Expressions.txt`, with their primary-expression grammar in `Syntax/Syntax/Expressions.txt` reusing the canonical constraint and type rules. Attach the new primary alternative once at `_PrimitiveExpr` so every expression consumer receives it.

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

**Implementation suggestion — Structural:** Store the template-head requires-clause on `GenericHeader` in `Syntax/Ast/Ast.txt` and the trailing requires-clause on `DeclaratorFunctionPart` in `Syntax/Ast/Types.txt`. Generalize `Syntax/Syntax/Generic.txt::_GenericHeader` and `DeclaratorComponents.txt::_DeclaratorFunctionPartOptionalParameters`, then let `Syntax/Syntax/Expressions.txt::_LambdaExpr` compose those shared pieces without lambda-only constraint productions.

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

**Implementation suggestion — Additive:** Add await/yield nodes in `Syntax/Ast/Expressions.txt`, a coroutine-return node in `Syntax/Ast/Statements.txt`, and `CoAwait` in `Syntax/Ast/QualifiedName.txt::Operators`. Own their grammar respectively in `Syntax/Syntax/Expressions.txt` at the unary/assignment-expression families, `Statements.txt` at the jump-statement family, and `QualifiedName.txt::_OperatorIdentifier`.

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

**Implementation suggestion — Structural:** Introduce one shared initializer-clause/braced-initializer representation in dedicated `Syntax/Ast/Initializers.txt` and `Syntax/Syntax/Initializers.txt` boxes, including an optional designator. Replace the parallel expression lists in `Syntax/Syntax/Expressions.txt` and `DeclarationVariable.txt::_VarBraceInit` with that rule so calls, braced expressions, variable initialization, and designated initialization keep one delimiter-owning structure.

## Array-Bound Deduction in New Expressions

C++20 permits an omitted first array bound when an initializer supplies the size. The current new-expression rule requires an expression in every pair of brackets. The rest of the full new-type-id gap is documented in [C++17 and Earlier Missing Features](Cases_17.md).

### Case No.1

```C++
new Type[]{first, second, third}
```

**Implementation suggestion — Structural:** Replace `Syntax/Ast/Expressions.txt::NewExpr.arrayArguments` with explicit new-declarator/bound nodes whose first bound can be absent, and remodel `Syntax/Syntax/Expressions.txt::_NewExpr` around the same declarator dimensions used by `DeclaratorComponents.txt`. An omitted bound must be represented structurally, not encoded by a special empty-expression patch.

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

**Implementation suggestion — Structural:** Add a canonical `InitStatement` AST/rule family to `Syntax/Ast/Statements.txt` and `Syntax/Syntax/Statements.txt`, covering expression, simple-declaration, and later alias-declaration forms. Make `_IfStatConditionPart`, classic `_ForStatConditionPart`, range-for, and `switch` consume that one family, and add an init field to the selection/iteration AST rather than another range-for-only declaration slot.

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

**Implementation suggestion — Additive:** Add one `StructuredBindingDeclaration` node to `Syntax/Ast/DeclsFuncVar.txt` and its declaration rule to `Syntax/Syntax/DeclarationVariable.txt`. Consume the structured-binding view of the shared decl-specifier sequence so `static` and `thread_local` reuse canonical specifier ordering without admitting every calling-convention or function-only keyword. Route that declaration through `Syntax/Syntax/Declarations.txt` and the block/range declaration consumers; storage duration should not require separate structured-binding node kinds.

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

**Implementation suggestion — Structural:** Generalize the shared declaration/function-specifier model in `Syntax/Ast/Types.txt` and the `_DeclaratorKeyword`/`_FunctionKeyword` families in `Syntax/Syntax/DeclaratorComponents.txt` to represent `consteval` in its two legal syntactic positions. Make `Syntax/Syntax/Expressions.txt::_LambdaExpr` consume the same function-suffix specifier sequence, rather than adding a lambda-only `consteval` production.

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

**Implementation suggestion — Structural:** Refine the declaration-specifier taxonomy in `Syntax/Ast/Types.txt` and `Syntax/Syntax/DeclaratorComponents.txt` so `constinit` is a declaration-only specifier that composes with the existing storage-class specifiers. All declaration paths in `Syntax/Syntax/DeclarationVariable.txt` should inherit it from that shared layer; putting it in a generic declarator-prefix bucket or patching individual variable rules would lose its placement information.

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

**Implementation suggestion — Structural:** Replace the bare `explicit` keyword entry with an `ExplicitSpecifier` AST node carrying an optional expression in `Syntax/Ast/Types.txt`, and parse it in the declaration-specifier family in `Syntax/Syntax/DeclaratorComponents.txt`. Constructor, conversion-function, and deduction-guide paths should all reference that node through their common declaration layer instead of receiving three parenthesized-expression patches.

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

**Implementation suggestion — Structural:** Change `Syntax/Ast/Decls.txt::NamespaceName` from a flat identifier path to components that each carry their own optional `inline` marker (and can later carry attributes). Then rewrite `Syntax/Syntax/DeclarationOthers.txt::_NsName`/`_NsDecl` around that component list, avoiding separate productions for every position at which `inline` can occur.

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

**Implementation suggestion — Structural:** Centralize operator-token-to-`Syntax/Ast/QualifiedName.txt::Operators` mapping in `Syntax/Syntax/QualifiedName.txt::_OperatorIdentifier` and make the comparison entry consume the lexer's single `<=>` token. The expression-operator family in `Syntax/Syntax/Expressions.txt` should reference the same mapping so declaration and expression spellings cannot drift again.

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

**Implementation suggestion — Additive:** Introduce generic `AttributeSpecifier`/attribute-token nodes and balanced-token grammar in dedicated `Syntax/Ast/Attributes.txt` and `Syntax/Syntax/Attributes.txt` boxes. Add attachment fields at the declaration, declarator, and statement owners in `Syntax/Ast/Decls*.txt`, `Types.txt`, and `Statements.txt`, and let their syntax boxes consume the shared attribute sequence; `no_unique_address`, `nodiscard`, `likely`, and `unlikely` should remain data, not dedicated productions.

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

**Implementation suggestion — Structural:** Add a `UsingEnumDeclaration` node beside the other using declarations in `Syntax/Ast/Decls.txt`, with a dedicated `_UsingEnumDecl` in `Syntax/Syntax/DeclarationOthers.txt`, and route it from `Syntax/Syntax/Declarations.txt` before the generic `_UsingValueDecl`. The explicit `enum` token provides a lookup-free structural discriminator, so the qualified-name parser can still be shared.

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
