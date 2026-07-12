# C++ Tokenizer Missing Features

This file audits `Test/Source/BuiltIn-Cpp/Syntax/Lexer.txt` against the lexical and preprocessing syntax needed by standard C++ through the C++26 working draft. It deliberately covers every supported language version: syntax removed from a later standard, such as trigraphs, still matters to the code indexer.

The important boundary is that `Lexer.txt` is a convenient phase-7 lexer, not a complete raw-source C++ frontend. The gaps divide into three groups:

1. Regular token definitions that are missing or incomplete and can be repaired in `Lexer.txt`.
2. Stateful source translation and token formation that need `RegexProc` or a dedicated C++ scanner.
3. Preprocessing operations that transform the token stream and therefore require a separate preprocessor.

The first group covers keyword/identifier policy, operator aliases, regular literal forms, suffixes, and generated Unicode ranges. The second begins where tokenization depends on captured delimiters, physical-line state, context, source decoding, maximal-munch exceptions, universal-character-name replacement, or phase-5 literal concatenation. The final `Preprocessing Requires a Separate Component` section is the third group. Sections such as digraphs, user-defined literals, Unicode, and C++26 basic characters deliberately describe both their declarative subset and the point where they cross into group 2.

Exact rejection of invalid text is not a goal. For example, assembling `>` `>` into a shift in the grammar is useful even though it also accepts `a > > b`. The cases below focus on accepting every valid spelling and retaining enough token identity for the syntax AST and later indexer passes.

## Missing Standard Keywords

The following unconditional keywords have no token declaration:

| First standard | Missing spellings |
| --- | --- |
| C++98 | `asm`, `export` |
| C++20 | `concept`, `consteval`, `constinit`, `co_await`, `co_return`, `co_yield`, `requires` |
| C++26 | `contract_assert` |

`__asm` is an implementation extension and is not a substitute for standard `asm`.

### Case No.1

```C++
asm("nop");

export template<typename T>
struct LegacyExportedTemplate;
```

### Case No.2

```C++
template<typename T>
concept C = true;

consteval int immediate()
{
    return 0;
}

constinit int initialized = 0;
```

### Case No.3

```C++
task coroutine()
{
    co_await ready;
    co_yield 42;
    co_return;
}

template<typename T>
    requires C<T>
struct Box;
```

### Case No.4

```C++
void check(int value)
{
    contract_assert(value >= 0);
}
```

## Over-Reserved Identifiers

`final` and `override` are identifiers with special meaning only in their grammar positions. `abstract` is not a standard special identifier at all. They are currently unconditional lexer tokens, so valid ordinary identifier uses are rejected before the parser can apply context.

`module`, `import`, `pre`, and `post` correctly remain identifiers and should be recognized with conditional literals or equivalent grammar rules.

### Case No.1

```C++
int abstract;
int final;
int override;

struct S
{
    void final();
    int override;
};
```

### Case No.2

```C++
int module;
int import;
int pre;
int post;
```

## Union-of-Versions Keyword Policy

The indexer must also accept a spelling as an identifier in editions before that spelling became a keyword. A practical union lexer should tokenize word-shaped spellings as `ID` and let the grammar use exact conditional literals. This intentionally permits some keyword-as-identifier spellings in newer code, which matches the requested over-accepting policy.

| Valid as an identifier before | Spellings that later became keywords |
| --- | --- |
| C++11 | `alignas`, `alignof`, `char16_t`, `char32_t`, `constexpr`, `decltype`, `noexcept`, `nullptr`, `static_assert`, `thread_local` |
| C++20 | `char8_t`, `concept`, `consteval`, `constinit`, `co_await`, `co_return`, `co_yield`, `requires` |
| C++26 | `contract_assert` |

### Case No.1

```C++
int constexpr;       // Valid before C++11.
int char8_t;         // Valid through C++17.
int concept;         // Valid through C++17.
int contract_assert; // Valid through C++23.
```

## Alternative Operator Spellings

All eleven standard word alternatives currently become `ID`: `and`, `and_eq`, `bitand`, `bitor`, `compl`, `not`, `not_eq`, `or`, `or_eq`, `xor`, and `xor_eq`. They can map to the corresponding punctuation token kinds or be conditional literals over `ID`.

### Case No.1

```C++
bool p = not a and b or c;
bool q = a not_eq b;
auto x = a bitand b;
auto y = a bitor b;
auto z = a xor b;
auto w = compl a;
```

### Case No.2

```C++
a and_eq b;
a or_eq b;
a xor_eq b;
```

## Digraph Punctuators

The standard digraphs are not canonicalized. The `<::` maximal-munch exception means that exact `<:` handling cannot be implemented safely by a global textual alias alone.

| Alternative | Primary token |
| --- | --- |
| `<%` | `{` |
| `%>` | `}` |
| `<:` | `[` |
| `:>` | `]` |
| `%:` | `#` |
| `%:%:` | `##` |

### Case No.1

```C++
int values<:2:> = <% 1, 2 %>;
```

### Case No.2

```C++
%:define CONCATENATE(a, b) a %:%: b
```

## Composite Punctuators

`Lexer.txt` has no dedicated token for `<=`, `>=`, `<<`, `>>`, `<<=`, `>>=`, or `##`. C++26 also adds `^^`, `[:`, and `:]`. The current grammar deliberately composes most comparison and shift operators from smaller tokens. That is sufficient for practical acceptance but loses adjacency and original-token identity.

Dedicated tokens are necessary only if later logic must distinguish one standard preprocessing token from separated characters. Otherwise the same practical decomposition can be extended to the C++26 punctuators.

### Case No.1

```C++
x <<= 1;
x >>= 1;
bool result = x <= upper && x >= lower;
```

### Case No.2

```C++
constexpr auto info = ^^int;
using Reflected = [:info:];
```

## C++26 Basic-Character Additions and Fallback Tokens

C++26 adds `$`, `@`, and the grave accent `` ` `` to the basic character set. They are valid raw-string delimiter characters. They can also be formed as the fallback single-character preprocessing-token category, which matters in skipped conditional groups even when no phase-7 token uses them. The current lexer recognizes none of the three.

### Case No.1

```C++
auto dollarDelimiter = R"$(text)$";
auto atDelimiter = R"@(text)@";
auto graveDelimiter = R"`(text)`";
```

### Case No.2

```C++
#if 0
$ @ `
#endif
```

## Digit Separators in Numeric Literals

`INT` accepts separators in decimal and octal spellings. The hexadecimal integer, binary integer, decimal floating, and hexadecimal floating regexes do not accept separators in their significands or exponents.

### Case No.1

```C++
auto hexadecimal = 0xDEAD'BEEF;
auto binary = 0b1010'0101;
```

### Case No.2

```C++
auto decimalFloat = 1'234.5'67e8'9;
auto hexadecimalFloat = 0x1'2.3'4p5'6;
```

## C++23 Integer Size Suffixes

The `z`/`Z` size suffix and all standard combinations with `u`/`U` are missing.

### Case No.1

```C++
1z;  1Z;
1uz; 1uZ; 1Uz; 1UZ;
1zu; 1zU; 1Zu; 1ZU;
```

## C++23 Extended Floating-Point Suffixes

The conditionally-supported suffixes `f16`, `F16`, `f32`, `F32`, `f64`, `F64`, `f128`, `F128`, `bf16`, and `BF16` are absent.

### Case No.1

```C++
auto f16 = 1.0f16;
auto f32 = 1.0F32;
auto f64 = 1.0f64;
auto f128 = 1.0F128;
auto bf16 = 1.0bf16;
auto BF16 = 1.0BF16;
```

## User-Defined Literal Tokens

The lexer does not form combined user-defined integer, floating, character, string, or raw-string literal tokens. A practical alternative is to consume a literal followed by an `ID` in the grammar and ignore adjacency. That accepts every valid form while deliberately also accepting whitespace-separated invalid forms.

Standard-library suffixes without a leading underscore must also be accepted; the underscore restriction is semantic and depends on which literal operator is declared.

### Case No.1

```C++
auto integer = 42_tag;
auto floating = 1.25_tag;
auto character = 'x'_tag;
auto string = "x"_tag;
auto raw = R"(x)"_tag;
```

### Case No.2

```C++
using namespace std::literals;
auto value = "text"s;
```

### Case No.3

Adjacent phase-5 string concatenation propagates the one user-defined suffix across the complete literal sequence, regardless of which component carries it.

```C++
auto suffixOnLast = "A" "B"_tag;
auto suffixOnFirst = "A"_tag "B";
auto repeatedSuffix = "A"_tag "B"_tag;
```

## Unicode Identifiers and Universal Character Names

`ID` is ASCII-only. Standard identifiers require direct Unicode `XID_Start`/`XID_Continue` characters and universal-character-name spellings. C++23 adds delimited and named forms.

Direct Unicode identifier characters are declaratively doable: VlppRegex processes Unicode scalar values, but `/w` is ASCII and has no Unicode property escape, so exact support requires generated `XID_Start` and `XID_Continue` range tables. Universal-character-name recognition, named-character lookup, replacement before token classification, and normalization checks are translation/scanner responsibilities. A deliberately broader non-ASCII rule followed by validation is another practical split.

### Case No.1

```C++
int café = 1;
int π = 2;
int \u03B1 = 3;
int \U000003B2 = 4;
```

### Case No.2

```C++
int \u{03B3} = 5;
int \N{GREEK SMALL LETTER DELTA} = 6;
```

## Preprocessing-Number Tokens

The current lexer immediately classifies complete language literals. Before macro expansion, C++ instead recognizes the broader `pp-number` language. A preprocessing number need not itself be a valid phase-7 numeric literal; token pasting or stringizing can still use it.

### Case No.1

```C++
#define STRINGIZE_IMPL(x) #x
#define STRINGIZE(x) STRINGIZE_IMPL(x)

const char* text = STRINGIZE(0xe+foo);
```

## Raw String Literals Require a Stateful Extension

All raw prefixes and delimiter forms are missing. The closing delimiter must equal the delimiter captured at the start and can be up to sixteen characters. The language is bounded and theoretically regular, but encoding every delimiter state in `Lexer.txt` is impractical; a backreference is not a pure VlppRegex expression and generated token definitions must pass `HasNoExtension()`.

`RegexProc::extendProc` is explicitly documented with a C++ raw-string example, so the overall library has an escape hatch. The generated `ParserBase::Tokenize` path does not install such a callback. A frontend using it must tokenize manually and then call a generated token-list parser overload, or use a dedicated C++ scanner for the whole lexical stage.

### Case No.1

```C++
auto ordinary = R"(a "quote" and \ backslash
second line)";

auto wide = LR"tag(text)tag";
auto utf8 = u8R"tag(text)tag";
auto utf16 = uR"tag(text)tag";
auto utf32 = UR"tag(text)tag";
```

### Case No.2

```C++
auto emptyDelimiter = R"(text)";
auto falseCloser = R"tag(text)other" more text)tag";
auto maxDelimiter = R"abcdefghijklmnop(text)abcdefghijklmnop";
auto suffixed = R"tag(text)tag"_suffix;
```

## Translation-Phase Line Splicing Requires a Scanner

A backslash followed by zero or more non-newline whitespace characters and then a newline is removed before preprocessing tokens are formed. Only the last backslash on a physical source line is eligible. This can join identifiers and string fragments, extend `//` comments, form a universal-character-name, and continue preprocessing directives. A nonempty file without a final newline also receives a synthesized newline. Raw strings require special handling because applicable early transformations are reverted within their content.

### Case No.1

```C++
int fo\
o = 1;

const char* text = "ab\
cd";
```

### Case No.2

```C++
#define ADD(a, b) ((a) + \
                   (b))

int first; // the comment continues \
int stillCommented;
int second;
```

### Case No.3

```C++
const char* raw = R"(a\
b)";
```

### Case No.4

The physical line before `u0061` ends in two backslashes. Only the second participates in splicing, leaving the first to begin the universal-character-name `\u0061`.

```C++
int \\
u0061 = 0;
```

### Case No.5

Store this one-line source fixture with no physical newline after the semicolon; phase 2 must synthesize it:

```C++
int finalLine = 0;
```

### Case No.6

The physical line ending after the backslash contains three U+0020 spaces before its newline. Splicing must still form the identifier `whitespace`. Use these exact UTF-8 bytes for the fixture so editors cannot trim the significant spaces:

```text
69 6E 74 20 77 68 69 74 65 5C 20 20 20 0A
73 70 61 63 65 20 3D 20 30 3B 0A
```

## Historical Trigraph Translation Requires a Scanner

C++14 and earlier replace trigraphs before token recognition. They were removed in C++17, but the union-of-versions indexer must retain them. In particular, `??/` becomes a backslash and can then participate in line splicing, so ordinary token aliases are not enough.

| Trigraph | Replacement |
| --- | --- |
| `??=` | `#` |
| `??/` | `\` |
| `??'` | `^` |
| `??(` | `[` |
| `??)` | `]` |
| `??!` | `|` |
| `??<` | `{` |
| `??>` | `}` |
| `??-` | `~` |

### Case No.1

```C++
??=define VALUE 1

int values??(2??) = ??<1, 2??>;
auto x = 1 ??' 2;
auto y = true ??! false;
auto z = ??-0;
```

### Case No.2

```C++
int fo??/
o = 0;
```

## Context-Sensitive Header Names Require a Scanner

A header-name token is formed only after specific occurrences of `include`, `embed`, `import`, `__has_include`, or `__has_embed`. Header contents can contain characters that are not otherwise valid phase-7 tokens. Globally matching `<...>` would steal template and comparison expressions.

### Case No.1

```C++
#include <library/header.hpp>

#if __has_include("optional/header.hpp")
#endif

import <library/header.hpp>;
```

### Case No.2

```C++
#embed "data.bin"

#if __has_embed(<optional/data.bin>)
#endif
```

## Maximal-Munch Exceptions Require Scanner Lookahead

The standard has exceptions to ordinary longest-token matching. The historical `<::` exception prevents `<:` from being selected in a template-like context. C++26 adds exceptions involving `[:` so an array bound followed by the `:>` digraph is not mistaken for a splice.

### Case No.1

```C++
template<typename T>
struct X;

X<::GlobalType> value;
```

### Case No.2

```C++
constexpr int N = 1;
int values[::N];

extern int incomplete[:>;
```

## Source Decoding and Normalization Require a Source Layer

Raw-source support also needs mandatory UTF-8 decoding, implementation-selected decoding for other accepted encodings, CRLF/CR normalization, leading BOM removal, Unicode scalar handling, and optional NFC/XID validation. `WString` input begins after much of this work has conceptually happened, so these are source-translation responsibilities rather than additional token regexes.

### Case No.1

```C++
// Store this source as UTF-8, with and without a leading BOM and with each
// supported physical newline convention.
int résumé = 0;
```

### Case No.2

The basic literal character set includes U+0000, so an exhaustive raw-source frontend must permit a physical null code point inside literal source text. A null-terminated `RegexLexer` walk cannot consume it. Create this fixture as binary UTF-8 source by replacing the marker with one U+0000 code point, not the two-character escape `\0`:

```C++
auto physicalNull = R"(before<U+0000>after)";
```

## Preprocessing Requires a Separate Component

The current lexer discards every newline as `SPACE`. It therefore cannot identify preprocessing directive boundaries. More fundamentally, preprocessing inserts, removes, duplicates, and synthesizes tokens; `RegexProc` can extend one token match but cannot implement those token-stream transformations.

A complete preprocessing stage must handle logical-line directive recognition, comments-as-space while preserving newlines, conditional inclusion, object-like and function-like macros, recursive rescanning, macro argument collection, `#`, `##`, placemarkers, `__VA_ARGS__`, `__VA_OPT__`, include/header lookup, module/import preprocessing, the null directive, `_Pragma`, predefined and feature-test macros, `__has_cpp_attribute`, `#embed` and its parameters, preprocessing constant expressions, and source provenance across expansion and token pasting.

### Case No.1

```C++
#include "header.hpp"
#define OBJECT value
#define FUNCTION(x) ((x) + 1)
#undef OBJECT

#if defined(FEATURE)
#elifdef ALTERNATIVE
#elifndef DISABLED
#else
#endif
```

### Case No.2

```C++
#line 100 "generated.cpp"
#pragma once

#define STRINGIZE(x) #x
#define CONCATENATE(a, b) a ## b
#define LOG(format, ...) log(format __VA_OPT__(,) __VA_ARGS__)
```

### Case No.3

```C++
#if 0
#error ignored error
#warning ignored warning
#endif

#embed "data.bin"
```

### Case No.4

```C++
_Pragma("once")

#if __has_cpp_attribute(nodiscard)
[[nodiscard]] int checked();
#endif

#if __cplusplus >= 202002L && __cpp_constexpr >= 201907L
constexpr int featureTest = 0;
#endif

#
```

### Case No.5

```C++
constexpr unsigned char data[] = {
#embed "data.bin" limit(16) prefix(0xAA,) suffix(, 0x55) if_empty(0)
};
```

## Already Covered or Intentionally Practical

The following are not tokenizer gaps under the requested policy:

- Decimal and octal integer digit separators are already accepted by `INT`.
- Binary literals and hexadecimal floating literals have token families, although their separator coverage is incomplete.
- Ordinary `u8`, `u`, `U`, and `L` character/string prefixes exist.
- C++23 delimited and named escape spellings inside ordinary strings and characters are swallowed by the deliberately broad escape regex. Exact escape validation is not needed for verified input.
- Standard comments are recognized. Only their interaction with translation phases and directive newlines remains missing.
- Split comparisons, shifts, and shift assignments already implement the desired practical behavior.
- `module`, `import`, `pre`, and `post` remaining `ID` tokens is desirable.

## Standards References

- [N4659 — C++17 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)
- [N4861 — C++20 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf)
- [N4950 — final C++23 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf)
- [N5046 — current C++26 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/n5046.pdf)
- [N4086 — removing trigraphs](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4086.html)
