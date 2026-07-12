# C++ 17 Missing Features

This file contains phase-7 syntax and AST gaps in `Test/Source/BuiltIn-Cpp` for C++17 and every earlier standard edition. Token formation and preprocessing are intentionally excluded and are covered by [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md).

Most sections describe spellings the current parser rejects. A few are called out as **AST fidelity gaps**: the token sequence is accepted, but the resulting generic node does not identify the standard construct well enough for a code indexer. Cases containing a complete declaration are intended for the `_File` entry; cases explicitly described as expressions are intended for `_TypeOrExpr` or `_Expr`.

The main missing families are nullable translation-unit/declaration forms, the standard attribute system, complete specifier/declarator forms, several expression and statement productions, namespace and class-head variants, and template declarations that do not use an ordinary template header.

Implementation notes use two categories:

- **Implementation suggestion — Additive:** the construct has no current AST or grammar representation, so it should be added to the named responsibility box.
- **Implementation suggestion — Structural:** the construct overlaps an existing category, so its canonical AST and rule family should be generalized for all consumers instead of patching one use site.

Prefer bounded practical over-acceptance when a small invalid superset substantially improves orthogonality and cannot misinterpret valid, compiler-verified input.

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

**Implementation suggestion — Additive:** Add an `EmptyDeclaration` node to `Syntax/Ast/Decls.txt` and route one public empty-declaration rule through `Syntax/Syntax/DeclarationOthers.txt` and `_DeclRejectSemicolon`, so namespace, class, and file declaration lists inherit bare `;` uniformly; block scope already treats `;` as `_OtherStats`'s empty statement. Keep `API.txt::_File` consuming one or more declarations because VlppParser2 rejects nullable complete clauses; handle a zero-token translation unit in the generated-parser facade by returning an empty `File` (or by consuming one private frontend sentinel). Both capabilities are absent additions and isolate the parser-runtime limitation without reorganizing existing declaration families.

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

**Implementation suggestion — Structural:** Replace the split between `Types.txt::_PrimitiveType` and position-sensitive keyword parsing in `DeclaratorComponents.txt` with one shared decl-specifier-sequence representation in `Syntax/Ast/Types.txt`. Parse atomic type, storage, function, CV, friend, and typedef specifiers in source order, then normalize fundamental-type combinations and declaration-wide properties once. Expose context views from that sequence—type-only, ordinary declaration, parameter, and class-member—so every consumer shares ordering logic while invalid specifier families can be excluded without cloning the grammar.

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

**Implementation suggestion — Additive:** Add shared attribute AST nodes in a dedicated `Syntax/Ast/Attributes.txt` and balanced-token rules in a matching `Syntax/Syntax/Attributes.txt`, exporting one `_AttributeSpecifierSeq` contract. Reference that contract from the owning declaration, name, declarator, statement, lambda, class, enum, namespace, and base-specifier productions instead of defining feature-specific attributes.

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

**Implementation suggestion — Structural:** Remove `alignas` from `DeclaratorComponents.txt::_AdvancedTypeNoCVNoMember` and model alignment specifiers alongside the shared declaration/type specifier sequence in `Syntax/Ast/Types.txt`. Give the operand one type-or-expression field plus an ellipsis token, and let declarations and class definitions attach a sequence of these specifiers before declarator assembly.

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

**Implementation suggestion — Structural:** Generalize `QualifiedName.txt` around a shared member-name/unqualified-id rule used after `::`, `.`, and `->`. Extend the identifier AST in `Syntax/Ast/QualifiedName.txt` for `~decltype(...)`, then make `Expressions.txt` consume that common member-name family rather than allowing destructor and conversion IDs only in `_NextLevelQualifiedName`.

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

**Implementation suggestion — Structural:** Redesign `Syntax/Ast/Expressions.txt::NewExpr` and `Syntax/Syntax/Expressions.txt::_NewExpr` around the standard alternatives: placement, an unparenthesized new-type-id, a parenthesized `_Type`, array bounds, and initializer. Reuse `_TypeBeforeDeclarator` with a new-specific abstract-declarator configuration from the declarator files instead of extending the current `_QualifiedName` plus bracket loop.

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

**Implementation suggestion — Structural:** Make the assignment-expression layer the canonical comma-free endpoint in `Expressions.txt` and let `_ThrowExpr` take that rule recursively, matching the standard grammar. Keep `_BExpr` as the sole comma wrapper so throw, conditional, and assignment operands do not acquire separate precedence implementations.

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

**Implementation suggestion — Structural:** Factor one braced-initializer-list rule and AST representation shared by `Expressions.txt` brace expressions/calls and `DeclarationVariable.txt::_VarBraceInit`. Put the optional trailing comma on that canonical list so every braced initializer consumer receives identical delimiter behavior.

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

**Implementation suggestion — Structural:** Introduce a shared defining-type-id view that combines the ordinary `API.txt::_Type` path with class and enum definitions, and change `UsingTypeDeclaration` in `Syntax/Ast/Decls.txt` to retain that common result. `DeclarationOthers.txt::_UsingTypeDecl` should consume this view instead of adding class/enum alternatives locally.

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

**Implementation suggestion — Structural:** Define one init-statement AST/rule family in `Syntax/Ast/Statements.txt` and `Syntax/Syntax/Statements.txt` that covers empty, expression, and standard simple-declaration forms. Make classic `for`, selection statements, and later range-for extensions reuse it rather than keeping `_ForStatConditionPart` branches tied to `_MultiVarsDecl`.

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

**Implementation suggestion — Structural:** Add a distinct init-statement field to both `IfElseStat` and `SwitchStat` in `Syntax/Ast/Statements.txt`, backed by the shared init-statement rule rather than the current `IfElseStat.varsDecl` special case. Rebuild `_IfStatConditionPart` and the switch production from `init-statement? + condition`, leaving condition ambiguity in `_ExprOrVarCondition`.

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

**Implementation suggestion — Additive:** Add a `constexprKeyword` token field to `IfElseStat` in `Syntax/Ast/Statements.txt` and recognize it in the common `Statements.txt::_IfStat` prefix before `(`. The existing condition and nearest-`else` machinery should remain unchanged.

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

**Implementation suggestion — Additive:** Add a structured-binding declaration node and binding-item list to `Syntax/Ast/DeclsFuncVar.txt`, with canonical rules in `Syntax/Syntax/DeclarationVariable.txt` for its specifiers, ref-qualifier, identifier list, and initializer. Export the declaration view to `Declarations.txt`, statement conditions, and `_ForEachParameter` so ordinary and range-for bindings share one representation.

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

**Implementation suggestion — Additive:** Add a `FoldExpr` node and fold-direction/operator representation to `Syntax/Ast/Expressions.txt`, then add one closed `_FoldOperator` family and the four fold shapes to `Syntax/Syntax/Expressions.txt` at the primary-expression level. Reuse the existing cast/prefix-expression operand layer rather than duplicating binary precedence rules.

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

**Implementation suggestion — Additive:** Add `NamespaceAliasDeclaration` to `Syntax/Ast/Decls.txt` and a public `_NsAliasDecl` to `Syntax/Syntax/DeclarationOthers.txt`, using the qualified namespace target representation and routing it through `Declarations.txt` with normal semicolon ownership.

## Inline Namespaces

Namespace definitions do not accept the leading `inline` keyword.

### Case No.1

```C++
inline namespace Version1
{
    int value;
}
```

**Implementation suggestion — Structural:** Generalize `NamespaceName` and `NamespaceDeclaration` in `Syntax/Ast/Decls.txt` into namespace components that retain an `inline` token, then rebuild `DeclarationOthers.txt::_NsDecl` from that component list. This also gives C++20 inline nested components the same owner instead of adding a one-off leading optional token.

## Root-Qualified Using Directives

`_UsingNsDecl` requires its first namespace component to be an identifier and cannot begin with `::`.

### Case No.1

```C++
namespace Services
{
}

using namespace ::Services;
```

**Implementation suggestion — Structural:** Replace `UsingNamespaceDeclaration.names` with a qualified namespace target that records root versus contextual scope and components, shared with namespace aliases. Make `DeclarationOthers.txt::_UsingNsDecl` consume that target rather than maintaining its own identifier-only path loop.

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

**Implementation suggestion — Structural:** Replace the single `UsingValueDeclaration.name` field in `Syntax/Ast/Decls.txt` with a list of `UsingDeclarator` items containing an optional `typename` token, a qualified name, and an optional ellipsis. Generalize `DeclarationOthers.txt::_UsingValueDecl` around one comma-separated item rule so ordinary, multiple, dependent, and pack-expanded using declarations remain one declaration family.

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

**Implementation suggestion — Additive:** After the tokenizer supplies the standard `asm` keyword, add `AsmDeclaration` to `Syntax/Ast/Decls.txt` and a public `_AsmDecl` to `Syntax/Syntax/DeclarationOthers.txt`. Route it through `Declarations.txt` so the same declaration node works at namespace and block scope.

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

**Implementation suggestion — Structural:** Factor a shared enum-key partial rule in `DeclarationOthers.txt` and use it from both `_EnumDecl` and `_EnumForwardDecl`. Map both `class` and `struct` scoped keys to the existing `EnumKind.EnumClass` representation so the synonymous spelling cannot drift between definitions and opaque declarations.

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

**Implementation suggestion — Structural:** Replace `EnumDeclaration.name : token` in `Syntax/Ast/DeclsEnum.txt` with a reusable enum-head-name/qualified-name representation, and factor one enum-head rule shared by full and opaque declarations in `DeclarationOthers.txt`. Keep enum key, qualified head, underlying type, and body as independent fields.

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

**Implementation suggestion — Structural:** Introduce a class-head representation in `Syntax/Ast/DeclsClass.txt` containing the class key, qualified name/template-id, and optional `final` token. Make `DeclarationClasses.txt::_ClassDecl` and `_ClassForwardDecl` reuse the applicable head components instead of extending the current `ID` plus separate generic-arguments fields.

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

**Implementation suggestion — Structural:** Add virtuality to `ClassInheritance` in `Syntax/Ast/DeclsClass.txt` and replace the duplicated first/second inheritance productions with one base-specifier core plus delimiter wrappers. That core should parse access and `virtual` in either standard order before the shared `_Type` base.

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

**Implementation suggestion — Structural:** Add a member-declarator/bit-field representation in `Syntax/Ast/DeclsFuncVar.txt` with an optional identifier and required width, and let class-member declaration rules use it. Do not weaken `_DeclaratorRequiredName` globally, because ordinary variables and non-member declarations must continue to require names.

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

**Implementation suggestion — Structural:** Make `friend` a declaration-wide property of the shared decl-specifier/declaration model instead of both a loose `_DeclaratorKeyword` and a separate narrow type rule. A canonical friend-declaration router should wrap either the common friend-type specifier path or the ordinary function/template declaration core, while preserving flexible specifier order. Reuse `_Type`/elaborated-type and existing declarator rules underneath; accepting `friend` in a few otherwise invalid specifier positions is a bounded superset that is preferable to duplicating every declaration family around the keyword.

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

**Implementation suggestion — Structural:** Split `DeclaratorFunctionPart` in `Syntax/Ast/Types.txt` into parameters-and-qualifiers, trailing return type, post-declarator virt-specifiers, and function-body specifier fields. Reorganize `DeclaratorComponents.txt::_DeclaratorFunctionPart` and the declaration-completion rules so `override`/`final`, pure specifiers, and `= delete`/`= default` attach after the complete declarator instead of living in one pre-arrow `_FunctionKeyword` list.

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

**Implementation suggestion — Structural:** Extract the catch-handler AST and syntax from statement-only `_TryStat` into a shared handler-sequence contract, then add ordinary-body and function-try-body variants to the function-definition representation in `Syntax/Ast/DeclsFuncVar.txt`. `DeclarationVariable.txt` should reuse the same constructor-initializer and block rules for both variants rather than copying statement `try` syntax.

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

**Implementation suggestion — Structural:** Change `VarStatInitItem.name` in `Syntax/Ast/DeclsFuncVar.txt` to a shared mem-initializer-id result and add an ellipsis field. Factor one `_MemInitializer` core in `DeclarationVariable.txt` that reuses qualified-name and `decltype` type syntax, with `:` and `,` handled only by list wrappers.

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

**Implementation suggestion — Additive:** Add `ExplicitInstantiationDeclaration` to `Syntax/Ast/Decls.txt` with an optional `extern` token and the instantiated declaration, then add dedicated need/reject-semicolon routes in `Syntax/Syntax/Declarations.txt` that begin with `template` without `_GenericHeader`. Keep this sibling to `TemplateDeclaration`, not an exception inside `_GenericHeader`.

## Typename Template-Template Parameter Keys

C++17 permits `typename` as the key after a template-template parameter's inner template header. The current grammar accepts only `class` there.

### Case No.1

```C++
template<template<typename> typename Container>
struct UsesContainer
{
};
```

**Implementation suggestion — Structural:** Factor a shared type-parameter-key rule for `class` and `typename` in `Generic.txt` and reuse it both for ordinary type parameters and after a template-template parameter's inner `_GenericHeader`. The existing `OrdinaryGenericParameter.typenameToken` field can retain either spelling.

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

**Implementation suggestion — Additive:** Add `DeductionGuideDeclaration` in `Syntax/Ast/DeclsFuncVar.txt` and reclassify the already accepted untyped-function-shaped declaration when its function part has the deduction-guide trailing return. Reuse `_DeclaratorUntypedFuncWithoutKeyword`, `_DeclaratorFunctionPart`, and the existing template/name AST unchanged; retain an ambiguity candidate only where the accepted spelling also has another viable untyped declaration interpretation.

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

**Implementation suggestion — Structural:** Add an export token/property to `TemplateDeclaration` in `Syntax/Ast/Decls.txt` and factor the ordinary template-declaration prefix in `Declarations.txt` so `export` composes only with the `_GenericHeader` form. Keep explicit specialization and explicit instantiation as separate template declaration families.

## Historical Auto Storage-Class Specifier

Before C++11, `auto` could be a storage-class specifier followed by an ordinary type. BuiltIn-Cpp always treats it as a placeholder type.

### Case No.1

```C++
void function()
{
    auto int value;
}
```

**Implementation suggestion — Structural:** Handle historical storage-class `auto` in the shared decl-specifier-sequence redesign owned by `Types.txt` and `DeclaratorComponents.txt`, not in `_QualifiedName`'s placeholder branch. The normalized sequence can treat `auto` followed by another type specifier as storage-class syntax while preserving placeholder `auto` when it supplies the type.

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

**Implementation suggestion — Additive:** Add `ClassAccessDeclaration` to `Syntax/Ast/DeclsClass.txt` and a `_ClassAccessDecl` rule to `Syntax/Syntax/DeclarationClasses.txt` using the shared qualified-name representation. Route it through the ordinary declaration router; accepting this otherwise invalid form outside class scope is a bounded superset that avoids a class-only routing exception and cannot alter valid compiler-verified input.

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

**Implementation suggestion — Structural:** Give the parameter-clause AST in `Syntax/Ast/Types.txt` an explicit C-style variadic-tail representation, including whether a comma token was present, and generalize `DeclaratorComponents.txt::_DeclaratorFunctionParameters` to accept both spellings. Keep the unnamed `int...` parse ambiguous with an abstract parameter-pack declarator instead of collapsing the two structures.

## Standards References

- [N4659 — C++17 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)
- [P0636R1 — changes between C++14 and C++17](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0636r1.html)
