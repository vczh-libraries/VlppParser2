# C++ Tokenizer Missing Features

## Feature and Case Index

- Missing Standard Keywords
  - Case No.1
  - Case No.2
  - Case No.3
  - Case No.4
- Over-Reserved Identifiers
  - Case No.1
  - Case No.2
- Union-of-Versions Keyword Policy
  - Case No.1
  - Case No.2 — Prefer Modern `auto`
- Alternative Operator Spellings
  - Case No.1
  - Case No.2
- Digraph Punctuators
  - Case No.1
  - ~~Case No.2 [WON'T FIX]~~
- Composite Punctuators
  - Case No.1
  - Case No.2
- C++26 Basic-Character Additions and Fallback Tokens
  - Case No.1
  - ~~Case No.2 [WON'T FIX]~~
- Digit Separators in Numeric Literals
  - Case No.1
  - Case No.2
- C++23 Integer Size Suffixes
  - Case No.1
- C++23 Extended Floating-Point Suffixes
  - Case No.1
- User-Defined Literal Tokens
  - Case No.1
  - Case No.2
  - Case No.3
- Unicode Identifiers and Universal Character Names
  - Case No.1
  - Case No.2
- ~~Preprocessing-Number Tokens [WON'T FIX]~~
  - ~~Case No.1 [WON'T FIX]~~
- Raw String Literals Require a Stateful Extension
  - Case No.1
  - Case No.2
- Translation-Phase Line Splicing Requires a Scanner
  - Case No.1
  - Case No.2
  - Case No.3
  - Case No.4
  - ~~Case No.5 [WON'T FIX]~~
  - Case No.6
- Historical Trigraph Translation Requires a Scanner
  - Case No.1
  - Case No.2
- Context-Sensitive Header Names Require a Scanner
  - Case No.1
  - ~~Case No.2 [WON'T FIX]~~
- Maximal-Munch Exceptions Require Scanner Lookahead
  - Case No.1
  - Case No.2
- Source Decoding and Normalization Require a Source Layer
  - Case No.1
  - ~~Case No.2 [WON'T FIX]~~
- ~~Preprocessing Requires a Separate Component [WON'T FIX]~~
  - ~~Case No.1 [WON'T FIX]~~
  - ~~Case No.2 [WON'T FIX]~~
  - ~~Case No.3 [WON'T FIX]~~
  - ~~Case No.4 [WON'T FIX]~~
  - ~~Case No.5 [WON'T FIX]~~

This file audits `Test/Source/BuiltIn-Cpp/Syntax/Lexer.txt` against the lexical forms useful to standard C++ through the C++26 working draft. It follows [C++ Syntax Implementation Philosophy](Philosophy.md): historically valid spellings matter to the code indexer, but compiler-precise rejection and preprocessing semantics do not.

The important boundary is that `Lexer.txt` is a convenient phase-7 lexer, not a complete raw-source C++ frontend. The gaps divide into three groups:

1. Regular token definitions that are missing or incomplete and can be repaired in `Lexer.txt`.
2. Stateful source translation and token formation that need `RegexProc` or a dedicated C++ scanner.
3. Preprocessing operations that transform the token stream and therefore remain outside BuiltIn-Cpp syntax.

The first group covers keyword/identifier policy, operator aliases, regular literal forms, suffixes, and permissive Unicode recognition. The second begins where tokenization depends on captured delimiters, physical-line state, context, source decoding, maximal-munch exceptions, or universal-character-name replacement. The final `Preprocessing Requires a Separate Component` section records the third group as **WON'T FIX** for this syntax project. A caller that needs macro-expanded input may still supply an externally prepared linear token list through the generated parser overload; that external tool is not part of BuiltIn-Cpp.

Exact rejection of invalid text is not a goal. For example, assembling `>` `>` into a shift in the grammar is useful even though it also accepts `a > > b`. The cases below focus on accepting valid source spellings with the smallest orthogonal token contract; token adjacency, provenance, Unicode normalization, and edition-specific branches are retained only when the index actually needs them.

Implementation notes use two categories:

- **Additive** means the capability is absent and can be added to the layer that owns it without reorganizing existing grammar contracts.
- **Structural** means a shared lexer, scanner, or syntax contract must be generalized. The recommendation updates the canonical path for every consumer instead of patching individual cases.

Both categories may use bounded practical over-acceptance: accept a small, documented invalid superset when that choice substantially improves orthogonality and cannot misinterpret compiler-verified valid input.

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

**Implementation suggestion — Structural:** Add one lexer token for each missing spelling, as `Lexer.txt` already does for other reserved words. Put `asm` in `Ast/Decls.txt` and `DeclarationOthers.txt::_AsmDecl` so block scope reaches it through declaration-statement routing; put `export` in template/module routing, `concept`/`requires` in a constraint box, `constinit` in the shared declaration-specifier family, `consteval` in function/lambda specifiers and consteval-block declarations, `co_await` in unary expressions and `QualifiedName.txt::_OperatorIdentifier`, `co_yield` at the assignment-expression layer, `co_return` in statements, and `contract_assert` in statements. Historical name use should reuse these same token kinds through the shared identifier rule when that does not introduce a competing modern parse; do not create edition-specific token streams.

## Over-Reserved Identifiers

`final` and `override` are identifiers with special meaning only in their grammar positions. `abstract` is not a standard special identifier at all. They are currently unconditional lexer tokens and `_NameIdentifier` accepts only `ID`, so valid ordinary identifier uses are rejected before the parser can apply context.

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

**Implementation suggestion — Structural:** Keep the dedicated tokens so grammar rules can request these spellings directly, and add them to the one `_NameIdentifier` family used by all name consumers. A post-declarator virt-specifier family should consume `final` and `override`, while the class-head family also consumes `final`; they should not remain in the generic pre-arrow `_FunctionKeyword` list. `abstract` stays available both to the explicitly supported extension rule and to `_NameIdentifier`. This shared-token design avoids both conditional-literal copies and context-specific identifier rules.

## Union-of-Versions Keyword Policy

The indexer should accept a spelling as an identifier in editions before that spelling became a keyword, unless doing so creates an unnecessary competing interpretation for modern syntax. Dedicated keyword tokens can still participate in the shared `_NameIdentifier` rule; they do not need to be downgraded to `ID` or branched by edition.

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

### Case No.2 — Prefer Modern `auto`

```C++
auto value = factory();
```

This has only the modern placeholder-type interpretation. BuiltIn-Cpp does not also treat `auto` as an identifier-shaped type, because that historical accommodation would add a duplicate declaration candidate.

**Implementation suggestion — Structural:** Add the version-changing keyword token kinds to the shared `_NameIdentifier` alternatives, then give every exact modern keyword construct token-only preference and exclude the identifier alternative whenever both would cover the same construct. `auto` is the canonical example, not the only exclusion. Outside such a collision, exact keyword syntax requests the dedicated token directly and historical name use continues through `_NameIdentifier`. This accepts historical names orthogonally without manufacturing keyword/name ambiguity or requiring an edition setting.

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

**Implementation suggestion — Additive:** Extend the existing punctuator token regexes with their word alternatives, so each spelling is emitted with the same generated token kind as its symbolic operator. For example, `AND_AND` owns both `&&` and `and`, and `AND_ASSIGN` owns both `&=` and `and_eq`. Longest-token matching keeps longer identifiers intact, while expression precedence, assignment, and `_OperatorIdentifier` continue to consume one canonical operator vocabulary with no scanner adapter or duplicated syntax branches.

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

### Case No.2 [WON'T FIX]

```C++
%:define CONCATENATE(a, b) a %:%: b
```

**Reason for not fixing Case No.2:** `%:` and `%:%:` are preprocessing operators. BuiltIn-Cpp does not implement directive execution, token pasting, or a preprocessing-token AST.

**Implementation suggestion — Structural:** Put the four phase-7 digraphs in the C++ scanner's punctuator-selection algorithm, where the `<::` exception is visible, and emit the existing `{`, `}`, `[`, or `]` token kind. The syntax grammar stays unaware of which equivalent spelling was used, and the index does not need original-spelling metadata. A raw-source wrapper may skip directive lines containing `%:` or `%:%:`; it must not route them into phase-7 syntax.

## Composite Punctuators

`Lexer.txt` has no dedicated token for `<=`, `>=`, `<<`, `>>`, `<<=`, `>>=`, or `##`. C++26 also adds `^^`, `[:`, and `:]`. The current grammar deliberately composes comparison and shift operators from smaller tokens. For an indexer this is the desired representation, not a fidelity gap.

The same practical decomposition can be extended to the C++26 punctuators. `##` belongs only to the preprocessing boundary marked **WON'T FIX** below.

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

**Implementation suggestion — Additive:** Keep comparisons and shifts exactly as the existing individual-token sequences consumed by `Expressions.txt`, `QualifiedName.txt`, and template closing. Define reflection and splice grammar rules with the equally orthogonal `^` `^`, `[` `:`, and `:` `]` sequences. This deliberately accepts separated spellings and needs no composite-token provenance. Do not add `##` to phase-7 grammar.

## C++26 Basic-Character Additions and Fallback Tokens

C++26 adds `$`, `@`, and the grave accent `` ` `` to the basic character set. They are valid raw-string delimiter characters. They can also be formed as the fallback single-character preprocessing-token category, which matters in skipped conditional groups even when no phase-7 token uses them. The current lexer recognizes none of the three.

### Case No.1

```C++
auto dollarDelimiter = R"$(text)$";
auto atDelimiter = R"@(text)@";
auto graveDelimiter = R"`(text)`";
```

### Case No.2 [WON'T FIX]

```C++
#if 0
$ @ `
#endif
```

**Reason for not fixing Case No.2:** The characters occur only in a skipped preprocessing group. BuiltIn-Cpp does not tokenize or evaluate conditional groups; a raw-source wrapper may skip the complete directive-controlled region or an external tool may supply prepared tokens.

**Implementation suggestion — Additive:** Extend only the raw-string scanner's delimiter classifier with `$`, `@`, and grave accent. Phase-7 syntax needs no general fallback-character token because no supported syntax construct consumes one.

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

**Implementation suggestion — Additive:** Factor separator-aware decimal, hexadecimal, and binary digit fragments plus separator-aware exponent digits in `Lexer.txt`. Reuse them from `INT`, `HEX`, `BIN`, `$CPP_FLOAT_DIGITS`, `$CPP_FLOATHEX_DIGITS`, and both exponent postfixes so every literal family follows one separator policy. Keep exact placement validation in the compiler-verification boundary rather than creating syntax productions per literal family.

## C++23 Integer Size Suffixes

The `z`/`Z` size suffix and all standard combinations with `u`/`U` are missing.

### Case No.1

```C++
1z;  1Z;
1uz; 1uZ; 1Uz; 1UZ;
1zu; 1zU; 1Zu; 1ZU;
```

**Implementation suggestion — Additive:** Add the `z`/`Z` suffix and its `u`/`U` orderings to the common integer-suffix fragment used by every integer token family in `Lexer.txt`. A single suffix definition should feed decimal, octal, hexadecimal, and binary literals instead of duplicating combinations across regexes.

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

**Implementation suggestion — Additive:** Extract a new `$CPP_FLOAT_SUFFIX` fragment containing the ordinary and conditionally supported fixed-width spellings, then reuse it from `$CPP_FLOAT_POSTFIX` and `$CPP_FLOATHEX_POSTFIX`. Preserve the decimal form's optional exponent and the hexadecimal form's mandatory `p` exponent; only the suffix is shared. The AST can continue to retain the literal spelling, with type support deferred.

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

The shared string-fragment representation should retain every suffix position without reproducing edition-specific concatenation validation.

```C++
auto suffixOnLast = "A" "B"_tag;
auto suffixOnFirst = "A"_tag "B";
auto repeatedSuffix = "A"_tag "B"_tag;
```

**Implementation suggestion — Structural:** Add one shared literal-suffix rule that consumes `_NameIdentifier` after any numeric, character, ordinary-string, or raw-string token and deliberately ignores adjacency. Let numeric/character literals use one small user-defined-literal wrapper, and let each existing `StringLiteralFragment` retain an optional suffix while `StringLiteral` continues to own the fragment sequence. This accepts whitespace-separated and repeatedly suffixed invalid forms, but it covers every valid literal uniformly without a phase-5 concatenation engine or edition-specific suffix propagation.

## Unicode Identifiers and Universal Character Names

`ID` is ASCII-only. Standard identifiers require direct Unicode `XID_Start`/`XID_Continue` characters and universal-character-name spellings. C++23 adds delimited and named forms.

VlppRegex processes Unicode scalar values, but `/w` is ASCII and has no Unicode property escape. Exact `XID_Start`/`XID_Continue`, named-character lookup, and NFC validation are compiler concerns. The indexer only needs one deliberately broader identifier path that accepts direct non-ASCII characters and all written universal-character-name forms.

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

**Implementation suggestion — Additive:** Extend one identifier matcher with a broad non-ASCII scalar range and permissive `\u`, `\U`, `\u{...}`, and `\N{...}` components. Do not resolve named characters or enforce XID/NFC constraints. Emit the existing `ID` token and keep its original token text. String, character, header-name, and raw-string matchers still own their complete spans, while `QualifiedName.txt` and every declarator-name rule remain unchanged because all identifier spellings reach the same contract.

## Preprocessing-Number Tokens [WON'T FIX]

The current lexer immediately classifies complete language literals. Before macro expansion, C++ instead recognizes the broader `pp-number` language. A preprocessing number need not itself be a valid phase-7 numeric literal; token pasting or stringizing can still use it.

### Case No.1 [WON'T FIX]

```C++
#define STRINGIZE_IMPL(x) #x
#define STRINGIZE(x) STRINGIZE_IMPL(x)

const char* text = STRINGIZE(0xe+foo);
```

**Reason for not fixing:** A `pp-number` exists to support macro argument collection, stringizing, token pasting, and rescanning. BuiltIn-Cpp begins from phase-7-shaped input and does not implement those preprocessing transformations. An external preprocessor may classify its surviving output into the existing literal token families before calling the prepared-token parser overload.

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

**Implementation suggestion — Additive:** Implement raw-string matching in the dedicated C++ scanner (or a `RegexProc`-backed tokenization adapter) and emit it through the same `StringLiteralFragment` contract used by ordinary strings. Delimiter capture and matching belong entirely to token formation; `_StringLiteralFragment` and the shared permissive literal-suffix rule should not duplicate raw-delimiter states.

## Translation-Phase Line Splicing Requires a Scanner

Through C++20, an immediately adjacent backslash-newline pair is removed before tokens are formed. C++23 and later also permit intervening non-newline whitespace. Splicing can join identifiers and string fragments, extend `//` comments, form a universal-character-name, and continue preprocessing directives. Raw strings require special handling because their contents retain the written characters.

### Case No.1

```C++
int fo\
o = 1;

const char* text = "ab\
cd";
```

### Case No.2

```C++
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

### Case No.5 [WON'T FIX]

Store this one-line source fixture with no physical newline after the semicolon; phase 2 must synthesize it:

```C++
int finalLine = 0;
```

**Reason for not fixing Case No.5:** The generated lexer and parser already accept end-of-input without a physical newline. Synthesizing a newline that is immediately discarded would add source-translation precision without changing the index.

### Case No.6

The physical line ending after the backslash contains three U+0020 spaces before its newline. Splicing must still form the identifier `whitespace`. Use these exact UTF-8 bytes for the fixture so editors cannot trim the significant spaces:

```text
69 6E 74 20 77 68 69 74 65 5C 20 20 20 0A
73 70 61 63 65 20 3D 20 30 3B 0A
```

**Implementation suggestion — Additive:** Add one permissive physical-line transformation before token recognition. Always remove a final backslash plus optional horizontal whitespace and the following newline, which accepts both historical and current spellings without edition branches. Protect raw-string spans, and retain only the physical range mapping needed for indexed tokens. Do not synthesize final newlines or run and merge separate edition-specific parses.

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

**Implementation suggestion — Additive:** Add one permissive trigraph translation pass ahead of line splicing and apply it for all input editions. This deliberately gives the historical spelling priority; modern compiler-verified code cannot depend on an adjacent trigraph sequence as a different phase-7 construct, and literal-content fidelity is not an indexing goal. Because `??/` feeds splicing, this remains a source transformation rather than a token alias. No edition branches, merged parses, or replacement provenance are required.

## Context-Sensitive Header Names Require a Scanner

A header-name token is formed only after specific occurrences of `include`, `embed`, `import`, `__has_include`, or `__has_embed`. Header contents can contain characters that are not otherwise valid phase-7 tokens. Globally matching `<...>` would steal template and comparison expressions.

### Case No.1

```C++
import <library/header.hpp>;
```

### Case No.2 [WON'T FIX]

```C++
#include <library/header.hpp>

#if __has_include("optional/header.hpp")
#endif

#embed "data.bin"

#if __has_embed(<optional/data.bin>)
#endif
```

**Reason for not fixing Case No.2:** `#include`, `#embed`, `__has_include`, and `__has_embed` require preprocessing handling and file lookup, which are outside BuiltIn-Cpp syntax.

**Implementation suggestion — Additive:** Add a narrow header-name scanner mode for the phase-7 `import <...>;` form and emit one permissive header token with the complete spelling. A raw-source wrapper may recognize and skip `#include`, `#embed`, and `__has_*` directive lines, but it should not expand macros, reconstruct header names, perform lookup, or evaluate the directives. An external preprocessor can instead supply prepared tokens when expanded input is required.

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

**Implementation suggestion — Structural:** Generalize the C++ scanner's punctuator chooser from unconditional longest match to an ordered decision table containing the standard exceptions. The same table should own digraphs, `<::`, and the C++26 `[:` boundaries and return the existing canonical token kinds. Keeping all exceptions in one chooser prevents template, array, and splice grammar rules from growing workarounds; original punctuator provenance is unnecessary for the index.

## Source Decoding and Normalization Require a Source Layer

Ordinary raw-source support needs UTF-8 decoding, CRLF/CR normalization, leading BOM removal, and Unicode scalar handling. `WString` input begins after this work has conceptually happened, so these are source-reader responsibilities rather than additional token regexes. Implementation-selected legacy encodings and NFC/XID validation are not required by the syntax indexer.

### Case No.1

```C++
// Store this source as UTF-8, with and without a leading BOM and with each
// supported physical newline convention.
int résumé = 0;
```

### Case No.2 [WON'T FIX]

The basic literal character set includes U+0000, so an exhaustive raw-source frontend must permit a physical null code point inside literal source text. A null-terminated `RegexLexer` walk cannot consume it. Create this fixture as binary UTF-8 source by replacing the marker with one U+0000 code point, not the two-character escape `\0`:

```C++
auto physicalNull = R"(before<U+0000>after)";
```

**Reason for not fixing Case No.2:** Supporting a physical U+0000 would require replacing the null-terminated lexer walk with a length-aware pipeline solely for literal-content fidelity. Literal contents are not indexed, so this compiler-precision edge case does not justify changing the parser contract.

**Implementation suggestion — Additive:** Keep a small byte-oriented wrapper outside BuiltIn-Cpp that decodes UTF-8, removes a leading BOM, normalizes physical newlines, and maps emitted token ranges back to byte offsets. It may reject embedded U+0000 and leave non-UTF-8 decoding to the caller. No normalization validator belongs in `Lexer.txt`.

## Preprocessing Requires a Separate Component [WON'T FIX]

The current lexer discards every newline as `SPACE`. It therefore cannot identify preprocessing directive boundaries. More fundamentally, preprocessing inserts, removes, duplicates, and synthesizes tokens; `RegexProc` can extend one token match but cannot implement those token-stream transformations.

A conforming preprocessing stage would need logical-line directive recognition, conditional inclusion, macros and recursive rescanning, `#`/`##`, inclusion and embed services, preprocessing expressions, and expansion provenance. Those operations implement compiler/build-environment behavior rather than BuiltIn-Cpp syntax.

### Case No.1 [WON'T FIX]

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

### Case No.2 [WON'T FIX]

```C++
#line 100 "generated.cpp"
#pragma once

#define STRINGIZE(x) #x
#define CONCATENATE(a, b) a ## b
#define LOG(format, ...) log(format __VA_OPT__(,) __VA_ARGS__)
```

### Case No.3 [WON'T FIX]

```C++
#if 0
#error ignored error
#warning ignored warning
#endif

#embed "data.bin"
```

### Case No.4 [WON'T FIX]

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

### Case No.5 [WON'T FIX]

```C++
constexpr unsigned char data[] = {
#embed "data.bin" limit(16) prefix(0xAA,) suffix(, 0x55) if_empty(0)
};
```

**Reason for not fixing:** BuiltIn-Cpp is a symbol-blind syntax indexer, not a compiler frontend. It will not own macro expansion, conditional selection, include/embed lookup, pragma execution, predefined macros, or preprocessing expression evaluation. A lightweight raw-source wrapper may skip complete directive lines when indexing written declarations. A build-aware caller that needs expanded code must use its own preprocessor and map the resulting linear token stream to generated C++ token IDs before calling the prepared-token parser overload. No syntax rule should insert, delete, paste, or rescan tokens.

## Already Covered or Intentionally Practical

The following are not tokenizer gaps under the requested policy:

- Decimal and octal integer digit separators are already accepted by `INT`.
- Binary literals and hexadecimal floating literals have token families, although their separator coverage is incomplete.
- Ordinary `u8`, `u`, `U`, and `L` character/string prefixes exist.
- C++23 delimited and named escape spellings inside ordinary strings and characters are swallowed by the deliberately broad escape regex. Exact escape validation is not needed for verified input.
- Standard comments are recognized. Their line-splicing interaction remains a scanner case; directive processing is intentionally outside scope.
- Split comparisons, shifts, and shift assignments already implement the desired practical behavior.
- `module`, `import`, `pre`, and `post` remaining `ID` tokens is desirable.
- Composite-token adjacency and original punctuator provenance are intentionally not retained.
- `auto` keeps only its preferred modern syntax roles instead of also entering the identifier/type path.

## Standards References

- [N4659 — C++17 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)
- [N4861 — C++20 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf)
- [N4950 — final C++23 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf)
- [N5046 — current C++26 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/n5046.pdf)
- [N4086 — removing trigraphs](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4086.html)
