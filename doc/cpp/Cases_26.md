# C++ 26 Missing Features

This file contains phase-7 syntax and AST gaps in the current C++26 working draft, N5046. Token formation for `^^`, `[:`, `:]`, `contract_assert`, raw strings, and `#embed` is covered separately in [Tokenizer and Preprocessor Gaps](Cases_Tokenizer.md).

C++26 adds several genuinely new grammar families: pack indexing, structured-binding extensions, variadic friends, new template-template parameter kinds, contracts, reflection and splicing, annotations, consteval blocks, and expansion statements. None has a corresponding BuiltIn-Cpp AST today. Semantic evaluation of reflection, constraints, or contracts is outside this audit; the parser only needs to retain their written structure and any syntactic ambiguity.

Implementation notes use two categories:

- **Implementation suggestion — Additive:** the construct is wholly absent and should be added to the AST and syntax box that owns its category.
- **Implementation suggestion — Structural:** an existing AST or rule family must be generalized or reorganized so all affected contexts share one orthogonal implementation.

Either category may use bounded practical over-acceptance when one shared rule replaces a large matrix of context-specific rules and cannot change the parse of valid input. Each such relaxation should state exactly which invalid combination is admitted; compiler-verified input, not the grammar, supplies the omitted validation.

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

**Implementation suggestion — Structural:** Add one `PackIndex : TypeOrExpr` node in `Syntax/Ast/Expressions.txt` and one public `_PackIndex` rule in `Syntax/Syntax/Expressions.txt`; make `_PostfixUnaryExpr` and `Syntax/Syntax/Types.txt`'s `_ShortTypeBeforeDeclarator` consume that same representation. Keeping the operand-plus-index spelling canonical avoids separate expression and type implementations and preserves the lookup-dependent interpretation; accepting a syntactically suitable operand without proving that it names a pack is the intended bounded semantic over-acceptance.

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

**Implementation suggestion — Additive:** Add `StructuredBindingDeclaration` and `StructuredBindingItem` to `Syntax/Ast/DeclsFuncVar.txt`, with the attribute list stored on each item, and add canonical `_StructuredBindingDecl` and `_StructuredBindingItem` rules to `Syntax/Syntax/DeclarationVariable.txt`. The item rule should consume the shared `_AttributeSpecifierSeq` owned by a dedicated `Syntax/Syntax/Attributes.txt` box rather than reproducing attribute syntax in the binding grammar.

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

**Implementation suggestion — Additive:** Give every `StructuredBindingItem` in `Syntax/Ast/DeclsFuncVar.txt` an optional pack marker and parse one uniform comma-separated item list in `Syntax/Syntax/DeclarationVariable.txt`. This deliberately also accepts multiple marked items; that bounded cardinality relaxation does not alter any valid parse and avoids separate before/middle/after list productions, while compiler-verified input guarantees the standard's single pack item.

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

**Implementation suggestion — Structural:** Generalize `_ExprOrVarCondition` in `Syntax/Syntax/DeclarationVariable.txt` into one condition router that accepts expressions, `VariablesDeclaration`, and the new `StructuredBindingDeclaration`, then use it from every declaration-capable condition consumer in `Syntax/Syntax/Statements.txt`. `ForStatLoopCondition.condition` in `Syntax/Ast/Statements.txt` must become `TypeOrExprOrOthers`, matching `IfElseStat`, `WhileStat`, and `SwitchStat`, so the classic-for condition is not patched separately.

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

**Implementation suggestion — Structural:** Keep one `DeclaratorKeyword` representation in `Syntax/Ast/Types.txt`, add `constinit` to the canonical keyword mapping in `Syntax/Syntax/DeclaratorComponents.txt`, and expose context-specific decl-specifier views over that mapping. The `_StructuredBindingDecl` rule in `Syntax/Syntax/DeclarationVariable.txt` should consume a shared sequence drawn from `static`, `thread_local`, `constexpr`, and `constinit`; deliberately accepting repeated or otherwise incompatible combinations is bounded to those four specifiers and keeps every valid ordering on one path.

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

**Implementation suggestion — Structural:** Split `_FunctionKeyword` in `Syntax/Syntax/DeclaratorComponents.txt` into ordered qualifier, exception-specifier, and function-body-specifier families, and represent the latter explicitly in `Syntax/Ast/Types.txt` instead of treating `= delete` like an undifferentiated keyword. The delete body-specifier node should retain its optional unevaluated string, while `_DeclaratorFunctionPart` composes the shared suffix families for ordinary functions and lambdas.

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

**Implementation suggestion — Structural:** Replace `FriendTypeDeclaration.type : QualifiedName` in `Syntax/Ast/Decls.txt` with a list of `FriendTypeItem` objects containing a `TypeOrExpr` and a pack marker. Generalize `_FriendTypeDecl` in `Syntax/Syntax/DeclarationOthers.txt` around one `_FriendTypeItem` rule that reuses the canonical type, elaborated-type, and dependent-`typename` paths, then parses the comma-separated list once.

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

**Implementation suggestion — Structural:** Generalize `OrdinaryGenericParameter` in `Syntax/Ast/Ast.txt` with an explicit template-parameter kind, and refactor `_GenericParameterKeyword` plus `_OrdinaryGenericParameter` in `Syntax/Syntax/Generic.txt` so `class`, `typename`, `auto`, and `concept` share the same inner header, optional pack, optional identifier, and default tail. This keeps parameter kind orthogonal to naming, packing, and defaulting instead of adding separate productions for every combination.

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

**Implementation suggestion — Additive:** Add `ContractAssertStat : Statement` with attributes and predicate fields to `Syntax/Ast/Statements.txt`, then add `_ContractAssertStat` to `Syntax/Syntax/Statements.txt` and route it once through `_Stat`. It should reuse `_AttributeSpecifierSeq` and `_Expr_NoComma`, producing the dedicated node even when the same tokens could be accepted as an identifier call.

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

**Implementation suggestion — Structural:** Add contract-specifier and postcondition-result-binding nodes to `Syntax/Ast/Types.txt`, including their attribute lists, and attach an ordered collection to `DeclaratorFunctionPart`. Refactor `_DeclaratorFunctionPart` and `_DeclaratorFunctionPartOptionalParameters` in `Syntax/Syntax/DeclaratorComponents.txt` to share one tail ordered as qualifiers, trailing return, optional requires-clause, then contracts; both ordinary functions and lambdas must reach the same `_ContractSpecifier` rule.

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

**Implementation suggestion — Additive:** Create a dedicated `Syntax/Ast/Reflection.txt` and `Syntax/Syntax/Reflection.txt` responsibility box containing `ReflectExpr : ExprOnly`, its operand representation, and public `_ReflectExpr`/`_ReflectOperand` rules. `_PrimitiveExpr` in `Syntax/Syntax/Expressions.txt` should consume that one entry; the AST should record `^^::` explicitly and use an ambiguity-enabled `TypeOrExpr` field for the reflection-name, type-id, and id-expression candidates.

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

**Implementation suggestion — Structural:** Factor all reflection operands through `_ReflectOperand` in `Syntax/Syntax/Reflection.txt` and apply longest-candidate priority or a syntax-only post-recognition filter at that boundary, not in `_QualifiedName`, `_Type`, or the relational-expression ladder. Parentheses should select an explicitly bounded operand branch, while the unparenthesized branch consumes the longest qualified/template/type-id form before normal expression parsing resumes.

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

**Implementation suggestion — Additive:** Let `Syntax/Ast/Reflection.txt` own distinct splice-specifier, template-splice, and specialization shapes, and let `Syntax/Syntax/Reflection.txt` expose one canonical `_SpliceExpr` family with optional specialization arguments represented by the existing `GenericArguments` contract. `_PrimitiveExpr` should add only that public family, leaving all later type, name, and declaration contexts to reuse the same splice nodes.

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

**Implementation suggestion — Structural:** Generalize `_PrimitiveType`/`_ShortTypeBeforeDeclarator` in `Syntax/Syntax/Types.txt` to consume the canonical splice-specifier and `typename` splice forms from `Syntax/Syntax/Reflection.txt`, with the result remaining a `TypeOrExpr`. Because `_ClassInheritanceFirst` and `_ClassInheritanceSecond` already consume `_Type`, base classes should receive the feature through that shared type path rather than through class-specific splice productions.

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

**Implementation suggestion — Structural:** Generalize the destructor identifier model in `Syntax/Ast/QualifiedName.txt` so it can retain either an ordinary token name or a computed type/splice, then extend the canonical `_DtorIdentifier` in `Syntax/Syntax/QualifiedName.txt`. `_MemberQualifiedNameFragment` in that file and `_DeclaratorUntypedFuncId` in `Syntax/Syntax/DeclaratorComponents.txt` must continue to reuse the single identifier rule, covering member access and destructor declarations together.

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

**Implementation suggestion — Structural:** Extend the fragment representation in `Syntax/Ast/QualifiedName.txt` and the `_QualifiedNameFragment`/`_NextLevelQualifiedName` families in `Syntax/Syntax/QualifiedName.txt` to admit canonical splice and template-splice fragments. Then keep `.` and `->` handling in `_PostfixUnaryExpr` pointed at `_QualifiedName`; duplicating splice-member alternatives in `Syntax/Syntax/Expressions.txt` would break the shared name topology.

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

**Implementation suggestion — Additive:** When the missing requires-expression family is added, give `Syntax/Ast/Expressions.txt` a requirement hierarchy with a dedicated splice type-requirement node and add `_SpliceTypeRequirement` beside the other requirement rules in `Syntax/Syntax/Expressions.txt`. That rule should consume the canonical `_SpliceSpecifier` or `_SpliceSpecialization` entry from `Syntax/Syntax/Reflection.txt`, so specialization arguments and splice delimiters are not reimplemented inside requires-expressions.

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

**Implementation suggestion — Structural:** Generalize the using-directive and namespace-alias target fields in `Syntax/Ast/Decls.txt` from token-only namespace-name lists to a shared qualified-name-or-splice representation, adding explicit namespace-alias and using-enum declaration nodes where they are absent. Refactor `_UsingNsDecl` and the namespace/using families in `Syntax/Syntax/DeclarationOthers.txt` around one canonical target rule, then route the new alias and using-enum declarations through `Syntax/Syntax/Declarations.txt`.

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

**Implementation suggestion — Additive:** Create `Syntax/Ast/Attributes.txt` and `Syntax/Syntax/Attributes.txt` as the single attribute responsibility box, with an annotation entry storing its constant expression and optional pack marker as an alternative to an ordinary attribute item. Expose `_AttributeSpecifierSeq` publicly and make every declaration, declarator, statement, and base-specifier attachment point consume it; deliberately leaving per-attribute subject restrictions to compiler validation is bounded to otherwise valid attribute attachment sites and prevents the reflection form from being copied into each owner.

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

**Implementation suggestion — Structural:** Add `ConstevalBlockDeclaration : Declaration` to `Syntax/Ast/Decls.txt` and `_ConstevalBlockDecl` to `Syntax/Syntax/DeclarationOthers.txt`, but also generalize declaration-statement routing in `Syntax/Syntax/Declarations.txt` and `Syntax/Syntax/Statements.txt`. A canonical block-scope declaration router should distinguish caller-owned semicolons from declarations that own a body, allowing the same consteval-block rule at namespace, class, and block scope without teaching `_ExprStat` a one-off exception.

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

**Implementation suggestion — Structural:** Add `ExpansionStat : Statement` to `Syntax/Ast/Statements.txt`, generalize `ForStatIterateCondition` into a shared range-control object, and factor reusable init-statement and range-declaration rules out of `_ForStatConditionPart` in `Syntax/Syntax/Statements.txt`. Ordinary range-for and `_ExpansionStat` should consume that same control, which in turn uses `_ForEachParameter`, the structured-binding declaration rule, and one canonical braced-initializer-list rule.

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

**Implementation suggestion — Additive:** Design the wholly absent `AsmDeclaration` in `Syntax/Ast/Decls.txt` with a nonempty payload of the generic balanced-token nodes owned by `Syntax/Ast/Attributes.txt`, then add `_AsmDecl` to `Syntax/Syntax/DeclarationOthers.txt` using the matching balanced-token rule from `Syntax/Syntax/Attributes.txt`. Route it through `Syntax/Syntax/Declarations.txt`; enumerating balanced leaves belongs in that shared token-tree family, not in an asm-only flat token list.

## Indeterminate Attribute

`[[indeterminate]]` is a new standard attribute for function parameters and uninitialized automatic block variables. It uses the generic attribute grammar but deserves version-specific coverage.

### Case No.1

```C++
void f([[indeterminate]] int parameter)
{
    int local [[indeterminate]];
}
```

**Implementation suggestion — Additive:** Represent `indeterminate` as an ordinary named item in the shared `Syntax/Ast/Attributes.txt`/`Syntax/Syntax/Attributes.txt` grammar; it needs no keyword-specific AST node. Connect `_AttributeSpecifierSeq` to the leading parameter and declarator attachment points owned by `Syntax/Syntax/DeclarationVariable.txt` and `Syntax/Syntax/DeclaratorConfigurations.txt`; accepting that name at other generic attribute hosts is an explicit bounded relaxation, since it neither changes valid parses nor requires a host-by-attribute matrix.

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

**Implementation suggestion — Structural:** Add the attribute list to the canonical label-payload hierarchy in `Syntax/Ast/Statements.txt` and make the shared `_LabelPayload` rule in `Syntax/Syntax/Statements.txt` consume `_AttributeSpecifierSeq` before the identifier. Ordinary labeled statements and `BlockStat`'s C++23 trailing payloads should reuse that node; contract result bindings obtain `maybe_unused` through their own result-binding node and the same attribute sequence.

## Already Covered or Practically Accepted

- A user-generated `static_assert` message is already accepted because `_StaticAssertDecl` permits any `_Expr_NoComma` as its message.
- The name-independent placeholder `_` is already an ordinary `ID`; its special declaration rules are semantic.
- The C++26 treatment of a comma-less trailing ellipsis does not add a new accepted spelling. The established cases remain in [C++17 and Earlier Missing Features](Cases_17.md), and the type-dependent interpretation belongs in [De-ambiguation Improvements](Cases_Improvement.md).
- Unevaluated-string changes mostly restrict already broad string acceptance, which does not require work under the practical policy.
- Trivial unions, constexpr exception handling, constexpr virtual inheritance, observable checkpoints, and most other constexpr changes are semantic.
- `#embed` and reflection punctuator formation belong in the tokenizer/preprocessor layer.

## Standards References

- [N5046 — current C++26 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/n5046.pdf)
- [P2893R3 — variadic friend declarations](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2893r3.html)
- [P2841R7 — concept and variable template-template parameters](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2841r7.pdf)
- [P2996R13 — static reflection](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r13.html)
- [P2361R6 — unevaluated strings and generalized asm payloads](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2361r6.pdf)
