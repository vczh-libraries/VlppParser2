# C++ 17 Missing Features

This file contains phase-7 syntax and AST gaps in `Test/Source/BuiltIn-Cpp` for C++17 and every earlier standard edition. Token formation and preprocessing are intentionally excluded and are covered by [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md).

Most sections describe spellings the current parser rejects. A few are called out as **AST fidelity gaps**: the token sequence is accepted, but the resulting generic node does not identify the standard construct well enough for a code indexer. Cases containing a complete declaration are intended for the `_File` entry; cases explicitly described as expressions are intended for `_TypeOrExpr` or `_Expr`.

The main missing families are nullable translation-unit/declaration forms, the standard attribute system, complete specifier/declarator forms, several expression and statement productions, namespace and class-head variants, and template declarations that do not use an ordinary template header.

## Empty Translation Units and Empty Declarations

`_File` requires at least one declaration, and no declaration rule accepts a bare semicolon. Empty declarations are permitted at namespace and class scope as well as in a translation unit.

### Case No.1

The input is a zero-token translation unit:

```C++
```

### Case No.2

```C++
;

namespace N
{
    ;
}

struct S
{
    ;
};
```

## Complete Decl-Specifier Ordering and Fundamental Type Sequences

BuiltIn-Cpp separates declaration keywords from the base type and generally requires non-CV keywords before the type. Standard decl-specifiers can be interleaved, and the fundamental type grammar permits valid combinations and orders not represented by `_PrimitiveType`.

### Case No.1

```C++
long unsigned int a;
short int b;
short unsigned int c;
long long int d;
unsigned long long int e;
int long long signed f;
double long g;
```

### Case No.2

```C++
signed static int object;
unsigned extern long externalObject;
signed inline long function();
int typedef Integer;

struct Owner
{
    signed friend int friendFunction();
};
```

## Standard Attributes

There is no attribute AST or grammar. Coverage must exercise declaration and name positions, declarator attachment points, statements and standalone attribute declarations, namespace/class/enum positions, pack expansion, attribute namespaces, and arbitrary balanced payloads.

### Case No.1

```C++
[[maybe_unused]] int object;
using OldName [[deprecated("use NewName")]] = int;

[[noreturn]] void terminateProgram();

struct [[nodiscard]] Result
{
    [[maybe_unused]] int member;
};
```

### Case No.2

```C++
enum class [[deprecated]] State
{
    oldValue [[deprecated]],
    ready,
};

namespace [[deprecated]] Legacy
{
}

[[deprecated]] void function([[maybe_unused]] int argument);
```

### Case No.3

```C++
struct Base
{
};

struct Derived : [[ ]] Base
{
    int [[ ]] value [[ ]];
    int* [[ ]] pointer;
    int array[4] [[ ]];
    int function() [[ ]];
    unsigned bit [[ ]] : 1;
};
```

### Case No.4

```C++
[[indexer::payload((1 + 2), {3, 4}, [5])]] int payload;
[[indexer::marker]];

int control(int value)
{
    switch (value)
    {
    case 0:
        [[fallthrough]];
    default:
        [[ ]] return value;
    }
}
```

### Case No.5

The lambda expression entry must retain a post-parameter declarator attribute and its balanced payload text:

```C++
[]() [[indexer::function_type((1), [2], {3})]] { return 0; }
```

### Case No.6

```C++
[[using indexer: first, second((1), [2], {3})]] int groupedAttributes;
```

### Case No.7

```C++
template<typename... Types>
[[indexer::for_types(Types)...]] void attributesFromPack();
```

## Alignment Specifiers

`alignas` currently behaves like a post-type declarator modifier. Standard placement before a declaration, a type-id operand, repeated alignment specifiers, and pack expansion are missing.

### Case No.1

```C++
alignas(32) int first;
alignas(double) unsigned char second;
alignas(8) alignas(16) unsigned char third[16];
```

### Case No.2

```C++
template<typename... Types>
struct Storage
{
    alignas(Types...) unsigned char bytes[64];
};
```

## Unqualified Conversion-Function and Pseudo-Destructor Names

`_OperatorTypeIdentifier` and `_DtorIdentifier` are available only after the grammar's `::` continuation. Direct member-access forms and `decltype` pseudo-destructors are absent.

### Case No.1

```C++
object.operator int()
```

### Case No.2

```C++
object.~Type()
```

### Case No.3

```C++
pointer->~Type()
```

### Case No.4

```C++
object.~decltype(value)()
```

## Complete New-Expression Type Syntax

`_NewExpr` accepts only a `_QualifiedName` followed by array bounds. Standard `type-specifier-seq`, CV-qualified and fundamental types, pointer new-declarators, and the parenthesized `type-id` alternative are missing.

### Case No.1

```C++
new int
```

### Case No.2

```C++
new const Type
```

### Case No.3

```C++
new Type*
```

### Case No.4

```C++
new Type*[count]
```

### Case No.5

```C++
new (Type)
```

### Case No.6

```C++
new (Type[count])
```

### Case No.7

```C++
new (storage) unsigned long[count]
```

## Complete Throw-Expression Operand

`_ThrowExpr` stops at `_BExpr10`, while the standard operand is an optional assignment-expression. Logical-or, conditional, and assignment operands are therefore missing.

### Case No.1

```C++
throw first || second
```

### Case No.2

```C++
throw condition ? first : second
```

### Case No.3

```C++
throw target = source
```

### Case No.4

```C++
throw throw value
```

## Trailing Commas in Braced Initializer Lists

The expression and variable brace rules do not permit an optional final comma.

### Case No.1

```C++
int values[] = {1, 2, 3,};

struct Point
{
    int x;
    int y;
};

Point point{1, 2,};
```

### Case No.2

```C++
Point{1, 2,}
```

## Alias Declarations with Defining Class or Enum Types

A standard `defining-type-id` may contain a class or enumeration definition. `_UsingTypeDecl` accepts only `_Type`, which has no defining class/enum branch.

### Case No.1

```C++
using Record = struct
{
    int value;
};
```

### Case No.2

```C++
using Choice = enum
{
    first,
    second,
};
```

## Complete Classic-For Init-Statements

Classic `for` accepts an expression or `_MultiVarsDecl`, not every standard simple-declaration. A typedef declaration is a compact regression case.

### Case No.1

```C++
void function()
{
    for (typedef int Integer; ; )
    {
        Integer value = 0;
        break;
    }
}
```

## C++17 Selection-Statement Initializers

`if` supports only an ordinary multi-variable declaration initializer. Expression and empty init-statements are missing, other simple-declarations are missing, and `switch` has no initializer branch at all.

### Case No.1

```C++
bool prepare();
bool ready();

void function()
{
    if (prepare(); ready())
    {
    }

    if (; ready())
    {
    }
}
```

### Case No.2

```C++
void function()
{
    if (typedef int Integer; true)
    {
        Integer number = 0;
    }
}
```

### Case No.3

```C++
int value();
bool prepare();

void function()
{
    switch (int selected = value(); selected)
    {
    default:
        break;
    }

    switch (prepare(); value())
    {
    default:
        break;
    }
}
```

## If-Constexpr Statements

The optional `constexpr` between `if` and `(` is absent.

### Case No.1

```C++
template<typename T>
int select(T value)
{
    if constexpr (sizeof(T) == 1)
        return 1;
    else
        return 2;
}
```

## Structured Bindings

There is no structured-binding declaration. Coverage should include copy and reference forms, all initializer spellings, and range-for declarations. Storage-duration specifiers on structured bindings are a C++20 extension and are covered in [C++20 Missing Features](Cases_20.md).

### Case No.1

```C++
struct Pair
{
    int first;
    int second;
};

Pair pair{};
auto [a, b] = pair;
auto& [c, d] = pair;
auto&& [e, f](Pair{});
```

### Case No.2

```C++
struct Pair
{
    int first;
    int second;
};

void iterate()
{
    Pair pairs[1]{};
    for (auto [x, y] : pairs)
    {
    }
}
```

## Fold Expressions

All unary and binary left/right fold shapes are missing.

### Case No.1

```C++
(arguments + ...)
```

### Case No.2

```C++
(... + arguments)
```

### Case No.3

```C++
(arguments + ... + initial)
```

### Case No.4

```C++
(initial + ... + arguments)
```

### Case No.5

The fold operator is a closed token set rather than an arbitrary binary operator. These expression statements cover every remaining token family; semantic operator applicability is intentionally outside syntax testing.

```C++
template<typename... Values>
void allFoldOperators(Values... values)
{
    (values .* ...);
    (values ->* ...);
    (values * ...);
    (values / ...);
    (values % ...);
    (values - ...);
    (values << ...);
    (values >> ...);
    (values < ...);
    (values > ...);
    (values <= ...);
    (values >= ...);
    (values == ...);
    (values != ...);
    (values & ...);
    (values ^ ...);
    (values | ...);
    (values && ...);
    (values || ...);
    (values , ...);
    (values = ...);
    (values += ...);
    (values -= ...);
    (values *= ...);
    (values /= ...);
    (values %= ...);
    (values ^= ...);
    (values &= ...);
    (values |= ...);
    (values <<= ...);
    (values >>= ...);
}
```

## Namespace Aliases

There is no namespace-alias declaration, including a root-qualified target.

### Case No.1

```C++
namespace Original
{
    namespace Nested
    {
    }
}

namespace Alias = ::Original::Nested;
```

## Inline Namespaces

Namespace definitions do not accept the leading `inline` keyword.

### Case No.1

```C++
inline namespace Version1
{
    int value;
}
```

## Root-Qualified Using Directives

`_UsingNsDecl` requires its first namespace component to be an identifier and cannot begin with `::`.

### Case No.1

```C++
namespace Services
{
}

using namespace ::Services;
```

## Using-Declarator Lists and Pack Expansion

The grammar accepts only one using target and has no using-declarator ellipsis.

### Case No.1

```C++
namespace First
{
    void function();
}

namespace Second
{
    int value;
}

using First::function, Second::value;
```

### Case No.2

```C++
template<typename... Bases>
struct Derived : Bases...
{
    using Bases::function...;
};
```

## Standard Asm Declarations

Standard `asm` is both a missing tokenizer keyword and a missing syntax production. This section records the grammar gap assuming the keyword token is available.

### Case No.1

```C++
asm("");

void function()
{
    asm("");
}
```

## Enum-Struct Declarations

The scoped-enum grammar accepts `enum class` but not the synonymous `enum struct` key.

### Case No.1

```C++
enum struct State
{
    ready,
};

enum struct Forward : unsigned;
```

## Qualified Enum Definitions

An enum head can use a nested-name-specifier when defining a previously declared member enum. The current grammar accepts only a single optional `ID`.

### Case No.1

```C++
struct Owner
{
    enum State : int;
};

enum Owner::State : int
{
    ready,
};
```

### Case No.2

```C++
template<typename T>
struct GenericOwner
{
    enum class State : int;
};

template<>
enum class GenericOwner<int>::State : int;
```

## Qualified Class Heads and Class Final

Class definitions accept only an optional unqualified identifier/template argument list and have no class `final` property.

### Case No.1

```C++
struct Outer
{
    struct Inner;
};

struct Outer::Inner final
{
};
```

## Virtual Base Specifiers

Base specifiers have no `virtual` branch. Both standard orders relative to an access specifier must be accepted.

### Case No.1

```C++
struct Base
{
};

struct First : virtual public Base
{
};

struct Second : public virtual Base
{
};
```

## Unnamed Bit-Fields

The bit-field branch requires a declarator identifier. Unnamed and zero-width padding fields are missing.

### Case No.1

```C++
struct Bits
{
    unsigned : 0;
    unsigned : 3;
};
```

## Complete Friend Type Forms

The friend-type rule takes only `_QualifiedName`. It excludes fundamental types and dependent `typename` forms. A non-class friend type is syntactically valid and is ignored semantically.

### Case No.1

```C++
template<typename T>
struct Owner
{
    friend int;
    friend typename T::Nested;
    friend typename T::template Rebind<int>;
};
```

### Case No.2

```C++
namespace Friends
{
    class Type;

    template<typename>
    class Template;
}

struct Owner
{
    friend class Friends::Type;
    friend class Friends::Template<int>;
};
```

## Function Suffixes After a Trailing Return Type

BuiltIn-Cpp stores every `_FunctionKeyword` before `->`. Standard virt-specifiers, pure-specifiers, and deleted/defaulted function bodies follow the complete declarator and therefore follow a trailing return type.

### Case No.1

```C++
struct Interface
{
    virtual auto first() -> int = 0;
    virtual auto second() -> int override;
    auto removed() -> int = delete;
    auto operator=(const Interface&) -> Interface& = default;
};
```

## Function-Try-Blocks

Only ordinary statement `try` blocks exist. Function definitions and constructors cannot place `try` before the function body/constructor initializer.

### Case No.1

```C++
int function() try
{
    return 0;
}
catch (...)
{
    return 1;
}
```

### Case No.2

```C++
struct Object
{
    int value;

    Object() try
        : value(0)
    {
    }
    catch (...)
    {
    }
};
```

## Complete Mem-Initializer-Ids and Pack Expansion

Constructor initializer names are restricted to `ID`. Qualified base names, `decltype` forms, template-ids, and the initializer ellipsis are missing.

### Case No.1

```C++
namespace Library
{
    struct Base
    {
        Base(int);
    };
}

struct Derived : Library::Base
{
    Derived()
        : Library::Base(0)
    {
    }
};
```

### Case No.2

```C++
template<typename... Bases>
struct Combined : Bases...
{
    Combined()
        : Bases()...
    {
    }
};
```

### Case No.3

```C++
struct Base
{
    Base();
};

Base base;

struct Derived : Base
{
    Derived()
        : decltype(base)()
    {
    }
};
```

## Explicit Template Instantiations

Every local `template` declaration requires `_GenericHeader`. Explicit instantiation has no header and optionally begins with `extern`.

### Case No.1

```C++
template<typename T>
struct Box
{
};

template struct Box<int>;
extern template struct Box<long>;
```

### Case No.2

```C++
template<typename T>
T identity(T value)
{
    return value;
}

template int identity<int>(int);
extern template long identity<long>(long);
```

## Typename Template-Template Parameter Keys

C++17 permits `typename` as the key after a template-template parameter's inner template header. The current grammar accepts only `class` there.

### Case No.1

```C++
template<template<typename> typename Container>
struct UsesContainer
{
};
```

## Deduction-Guide AST Classification

This is an **AST fidelity gap**. A deduction-guide spelling can be accepted as a generic untyped function declaration with a trailing return type, but the AST does not identify a deduction guide. The arrow and matching template-name shape allow a syntax-only postpass to reclassify it.

### Case No.1

```C++
template<typename T>
struct Box
{
    Box(T);
};

template<typename T>
Box(T) -> Box<T>;

Box(const char*) -> Box<const char*>;
```

## Historical Exported Templates

The indexer must retain the C++98/C++03 `export template` spelling even though it was removed from later standards.

### Case No.1

```C++
export template<typename T>
T identity(T value)
{
    return value;
}
```

## Historical Auto Storage-Class Specifier

Before C++11, `auto` could be a storage-class specifier followed by an ordinary type. BuiltIn-Cpp always treats it as a placeholder type.

### Case No.1

```C++
void function()
{
    auto int value;
}
```

## Historical Class Access Declarations

Pre-C++11 access declarations used a qualified member name without `using`. They were replaced by using-declarations but remain part of the union-of-versions input language.

### Case No.1

```C++
struct Base
{
    void function();
};

struct Derived : Base
{
    Base::function;
};
```

## Comma-Less Variadic Parameters

The standard retains the deprecated `parameter-declaration-list ...` alternative in addition to the comma form. The current parser rejects a named final parameter followed immediately by the ellipsis. An unnamed non-placeholder spelling can be accepted with a pack-shaped AST, which is an AST fidelity issue for a union-of-versions indexer.

### Case No.1

```C++
void log(const char* format...);
```

### Case No.2

```C++
void function(int...);
```

## Standards References

- [N4659 — C++17 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)
- [P0636R1 — changes between C++14 and C++17](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0636r1.html)
