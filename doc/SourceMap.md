# Source map

This map answers two questions for every area under [`Source`](../Source): where a responsibility lives, and which representation that code expects. It intentionally groups generated files by artifact family because they are outputs of the generator and are not hand-maintained implementation modules.

The recommended navigation rule is to enter through a header or orchestration file, then follow the phase-specific `.cpp` file. Many large algorithms are split by phase rather than by class; reading all files with the same prefix is necessary before changing an invariant.

## Root runtime foundations

| Files | Responsibility | Read when |
| --- | --- | --- |
| [`AstBase.h`](../Source/AstBase.h), [`AstBase.cpp`](../Source/AstBase.cpp) | Source positions/ranges, `ParsingAstBase`, `ParsingToken`, builder/visitor bases, JSON string helpers, the `AstIns` instruction set, `IAstInsReceiver`, the generic slot/object VM, generated-assembler templates, reflection registration, and embedded-data decompression. | Changing generated AST contracts, AST instruction semantics, field assignment, ambiguity wrappers, or source ranges. |
| [`AstPrint.h`](../Source/AstPrint.h), [`AstPrint.cpp`](../Source/AstPrint.cpp) | `ParsingWriter` plus recorder strategies for original, generated, updated, or fan-out node ranges while printing an AST. | Adding a source-preserving/canonical printer or tracking generated locations. |
| [`Executable.h`](../Source/Executable.h), [`Executable.cpp`](../Source/Executable.cpp) | Pointer-free PDA descriptors, flat-array slice types, serialization schema, metadata, `IExecutor`, ambiguity exception, and executor factory declaration. | Changing the generated binary contract or runtime input/return representation. |
| [`SyntaxBase.h`](../Source/SyntaxBase.h), [`SyntaxBase.cpp`](../Source/SyntaxBase.cpp) | `ParserBase` generated-parser facade: load lexer/parser data, tokenize, filter discarded tokens, drive the executor phases, expose events, report errors, and cast the root AST. | Changing public generated parser behavior or orchestration between lexer, executor, and AST receiver. |

These files are present in the runtime-only release. They do not depend on parser-definition ASTs or generator symbol managers.

## AST semantic model and generation

### Symbols and bootstrap definitions

| File | Responsibility |
| --- | --- |
| [`Ast/AstSymbol.h`](../Source/Ast/AstSymbol.h) | Declares AST symbol ownership: manager → file group → file → enum/class → item/property; visibility, inheritance, ambiguity helper types, lookup helpers, and bootstrap entry points. |
| [`Ast/AstSymbol.cpp`](../Source/Ast/AstSymbol.cpp) | Implements duplicate detection, type/base resolution, visibility and cycle checks, `ToResolve`/`Common` generation, common-base lookup, inherited-property lookup, and group/file registration. |
| [`Ast/AstSymbol_CreateParserGenTypeAst.cpp`](../Source/Ast/AstSymbol_CreateParserGenTypeAst.cpp) | Constructs the parser generator's AST-definition AST model directly in C++ for bootstrap. |
| [`Ast/AstSymbol_CreateParserGenRuleAst.cpp`](../Source/Ast/AstSymbol_CreateParserGenRuleAst.cpp) | Constructs the parser generator's syntax-definition AST model—conditions, switches, syntax nodes, clause forms, assignments, and rules—directly in C++. |

### C++ generation coordinator and shared templates

| File | Responsibility |
| --- | --- |
| [`Ast/AstCppGen.h`](../Source/Ast/AstCppGen.h) | Declares filename generation and all AST/utility/assembler writer entry points. |
| [`Ast/AstCppGen.cpp`](../Source/Ast/AstCppGen.cpp) | Coordinates filenames and artifact emission; writes common includes, guards, namespace scaffolding, and the parser-wide assembler after group artifacts. |
| [`Ast/AstCppGen_Classes.cpp`](../Source/Ast/AstCppGen_Classes.cpp) | Emits enums, AST class declarations/implementations, fields, visitor interfaces, `Accept`, reflection type information, proxies, and type loaders. |
| [`Ast/AstCppGen_Assembler.cpp`](../Source/Ast/AstCppGen_Assembler.cpp) | Assigns parser-wide numeric class/field IDs and emits the generated `AstInsReceiverBase` subclass, construction/setter switches, name lookup, and ambiguity factories. |
| [`Ast/AstCppGen_Builder.cpp`](../Source/Ast/AstCppGen_Builder.cpp) | Emits fluent `MakeType` builders over `ParsingAstBuilder<T>`. |
| [`Ast/AstCppGen_EmptyVisitor.cpp`](../Source/Ast/AstCppGen_EmptyVisitor.cpp) | Emits no-op visitor bases for selective overrides. |
| [`Ast/AstCppGen_CopyVisitor.cpp`](../Source/Ast/AstCppGen_CopyVisitor.cpp) | Emits deep-copy traversal for tokens, objects, arrays, enums, and source ranges. |
| [`Ast/AstCppGen_TraverseVisitor.cpp`](../Source/Ast/AstCppGen_TraverseVisitor.cpp) | Emits generic traversal hooks in base-to-derived order and finishing hooks in reverse order. |
| [`Ast/AstCppGen_JsonVisitor.cpp`](../Source/Ast/AstCppGen_JsonVisitor.cpp) | Emits AST-to-JSON writers, compact JSON-to-AST readers, and TypeScript declarations. Reader generation owns direct type/enum/field dispatch, inherited filling, recursive typed conversion, validation, and missing-field defaults. |

The assembler writer is a stage boundary, not only a text emitter: it fills `CppParserGenOutput::classIds` and `fieldIds`, which syntax lowering consumes later.

## Lexer semantic model and generation

| File | Responsibility |
| --- | --- |
| [`Lexer/LexerSymbol.h`](../Source/Lexer/LexerSymbol.h) | Declares ordered token symbols, regex/display/discard metadata, name and fixed-text maps, and the bootstrap lexer factory. |
| [`Lexer/LexerSymbol.cpp`](../Source/Lexer/LexerSymbol.cpp) | Parses individual regex ASTs for validation, rejects non-pure lexer regex extensions, detects fixed literal text, and reports duplicate names/display text. |
| [`Lexer/LexerSymbol_CreateParserGenLexer.cpp`](../Source/Lexer/LexerSymbol_CreateParserGenLexer.cpp) | Constructs the common lexer used by the AST-definition and syntax-definition parsers. |
| [`Lexer/LexerCppGen.h`](../Source/Lexer/LexerCppGen.h), [`Lexer/LexerCppGen.cpp`](../Source/Lexer/LexerCppGen.cpp) | Emit token enums/metadata/deleter, build and serialize the combined `RegexLexer`, and embed its compressed data. |

The line-oriented definition reader itself is [`ParserGen/CompileLexer.cpp`](../Source/ParserGen/CompileLexer.cpp), not part of `LexerSymbolManager`.

## Shared generator policy and output metadata

| File | Responsibility |
| --- | --- |
| [`ParserGen_Global/ParserSymbol.h`](../Source/ParserGen_Global/ParserSymbol.h) | `MappedOwning<T>`, parser-wide configuration, structured error inventory, locations, output-definition kinds, and common helpers. |
| [`ParserGen_Global/ParserSymbol.cpp`](../Source/ParserGen_Global/ParserSymbol.cpp) | Initializes the bootstrap parser generator's name, includes, namespace, and guard. |
| [`ParserGen_Global/ParserCppGen.h`](../Source/ParserGen_Global/ParserCppGen.h) | Output manifests for AST/syntax/parser artifacts plus the class/field/token ID dictionaries shared across stages. |
| [`ParserGen_Global/ParserCppGen.cpp`](../Source/ParserGen_Global/ParserCppGen.cpp) | Common file/namespace/string writers and compressed binary-to-C++ embedding in 256-byte `\xHH` rows. |

The full production ordering of these APIs lives in the non-test entry point [`Tools/GlrParserGen/GlrParserGen/Main.cpp`](../Tools/GlrParserGen/GlrParserGen/Main.cpp).

## Parser-definition compilation

### Entry points and shared context

| File | Responsibility |
| --- | --- |
| [`ParserGen/Compiler.h`](../Source/ParserGen/Compiler.h) | Declares AST/lexer/syntax compilation entry points and `VisitorContext`/`VisitorSwitchContext`, including all node-to-symbol and dependency maps shared among passes. |
| [`ParserGen/CompileAst.cpp`](../Source/ParserGen/CompileAst.cpp) | Two-pass AST definition lowering: declare every enum/class and ambiguity helper, then fill items/bases/properties after forward references are available. |
| [`ParserGen/CompileLexer.cpp`](../Source/ParserGen/CompileLexer.cpp) | Handwritten line parser for tokens, discarded tokens, comments, fragments, and textual fragment expansion. Also implements quoted-literal delimiter removal. |
| [`ParserGen/CompileSyntax.cpp`](../Source/ParserGen/CompileSyntax.cpp) | Merges syntax files, creates all rule symbols, orders semantic passes, performs optional switch rewriting and revalidation, then invokes lowering. |

### Syntax semantic passes

| File | Responsibility |
| --- | --- |
| [`ParserGen/CompileSyntax_ResolveName.cpp`](../Source/ParserGen/CompileSyntax_ResolveName.cpp) | Resolves AST types, token/rule references, ordinary/conditional literals, `!Rule` dependencies, cross-file visibility, and switch names. |
| [`ParserGen/CompileSyntax_ValidateSwitchesAndConditions.cpp`](../Source/ParserGen/CompileSyntax_ValidateSwitchesAndConditions.cpp) | Finds direct and transitive switch influence, accounts for pushed overrides, validates observed pushed values, and identifies unaffected expansion roots. |
| [`ParserGen/CompileSyntax_CalculateTypes.cpp`](../Source/ParserGen/CompileSyntax_CalculateTypes.cpp) | Validates all-partial rules, infers rule and reuse-clause types through dependency SCCs, and enforces compatible common bases. |
| [`ParserGen/CompileSyntax_RewriteSyntax_Switch.cpp`](../Source/ParserGen/CompileSyntax_RewriteSyntax_Switch.cpp) | Partially evaluates switches into specialized rules, applies nested values, simplifies empty paths, creates combined subset helpers, and replaces affected symbols. |
| [`ParserGen/CompileSyntax_CalculateFirstSet.cpp`](../Source/ParserGen/CompileSyntax_CalculateFirstSet.cpp) | Presently performs only a non-persisted could-be-empty traversal; it is not the source of the start sets used by prefix merge. |
| [`ParserGen/CompileSyntax_ValidateTypes.cpp`](../Source/ParserGen/CompileSyntax_ValidateTypes.cpp) | Verifies field/value kinds, rule covariance, partial/reuse placement, and enum assignments; records scalar-field assignment properties. |
| [`ParserGen/CompileSyntax_ValidateStructure.cpp`](../Source/ParserGen/CompileSyntax_ValidateStructure.cpp) | Counts minimum consumption and reuse occurrences, validates loop/optional/priority constraints, checks scalar field multiplicity, and rejects recursive partial-rule expansion. |
| [`ParserGen/CompileSyntax_CompileSyntax.cpp`](../Source/ParserGen/CompileSyntax_CompileSyntax.cpp) | Converts validated `GlrSyntax` and clause AST nodes into `AutomatonBuilder` operations and numeric AST instructions. |

Read these in the schedule defined by [`CompileSyntax.cpp`](../Source/ParserGen/CompileSyntax.cpp), not merely in filename order. With switches, the first context runs name resolution, switch validation, partial-rule validation, and type calculation before rewriting. A fresh context then repeats those four stages on the rewritten grammar before the inert `CalculateFirstSet` hook, field/type validation, structural validation, and lowering. Without switches, only that second sequence runs. Rebuilding the context is required because rewriting replaces definition-AST nodes and rule symbols used as map keys.

## Definition printers

| File | Responsibility |
| --- | --- |
| [`ParserGen_Printer/AstToCode.h`](../Source/ParserGen_Printer/AstToCode.h) | Declares semantic-symbol-to-definition-AST and canonical AST/syntax text printers. |
| [`ParserGen_Printer/TypeSymbolToAst.cpp`](../Source/ParserGen_Printer/TypeSymbolToAst.cpp) | Reconstructs `GlrAstFile` from AST symbols; can hide generated ambiguity helper classes and restore the declarative form. |
| [`ParserGen_Printer/TypeAstToCode.cpp`](../Source/ParserGen_Printer/TypeAstToCode.cpp) | Prints AST definition trees in canonical syntax. |
| [`ParserGen_Printer/SyntaxAstToCode.cpp`](../Source/ParserGen_Printer/SyntaxAstToCode.cpp) | Precedence-aware canonical syntax printer for conditions, EBNF nodes, switches, clauses, and assignments. |

These printers support diagnostics, rewritten-grammar inspection, and round-trip comparison. They are not on the recognition hot path.

## Mutable syntax automaton

### Core IR and builder

| File | Responsibility |
| --- | --- |
| [`Syntax/SyntaxSymbol.h`](../Source/Syntax/SyntaxSymbol.h) | Declares rules, states, edges, input kinds, competitions, syntax phases, prefix-merge solution records, manager graph APIs, and bootstrap syntax factories. |
| [`Syntax/SyntaxSymbol.cpp`](../Source/Syntax/SyntaxSymbol.cpp) | Implements graph ownership, rule creation/removal, strict phase transitions, stable state/edge ordering, and diagnostic state labels. |
| [`Syntax/SyntaxSymbolWriter.h`](../Source/Syntax/SyntaxSymbolWriter.h) | Declares `AutomatonBuilder` and the strongly typed C++ `syntax_writer` DSL used for bootstrap grammars. |
| [`Syntax/SyntaxSymbolWriter.cpp`](../Source/Syntax/SyntaxSymbolWriter.cpp) | Builds Thompson-style clause fragments, numbered slots, deferred fields, create/reuse envelopes, and optional priority competitions. |

### Compact-NFA phases

| File | Responsibility |
| --- | --- |
| [`Syntax/SyntaxSymbol_NFACompact.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact.cpp) | Coordinates per-rule epsilon removal, direct-left-recursion elimination, exact merges, same-rule factoring, then global prefix solving/application; manages incremental graph replacement. |
| [`Syntax/SyntaxSymbol_NFACompact_EliminateEpsilonEdges.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact_EliminateEpsilonEdges.cpp) | Computes epsilon closures, concatenates instructions/competitions onto direct token/rule edges, joins clause starts, and creates explicit ending edges. |
| [`Syntax/SyntaxSymbol_NFACompact_EliminateLeftRecursion.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact_EliminateLeftRecursion.cpp) | Converts direct start-state self calls into post-prefix `LeftRec` continuations. |
| [`Syntax/SyntaxSymbol_NFACompact_MergeEdgesWithSameInput.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact_MergeEdgesWithSameInput.cpp) | Subset-style target-state merging for edges whose input, instructions, and competitions are identical. |
| [`Syntax/SyntaxSymbol_NFACompact_MergeEdgesWithSameRuleUsingLeftrec.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact_MergeEdgesWithSameRuleUsingLeftrec.cpp) | Factors one common rule call when caller-specific continuation actions differ, representing those actions as `LeftRec` continuations. |
| [`Syntax/SyntaxSymbol_NFACompact_PrefixMergeCrossReference.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact_PrefixMergeCrossReference.cpp) | Builds transitive token/rule start-set bitsets, rejects indirect left recursion, solves minimal non-overlapping shared prefixes at every state, and injects rule/token fronts plus simulated continuations. |

### Final lowering and C++ parser output

| File | Responsibility |
| --- | --- |
| [`Syntax/SyntaxSymbol_NFACrossReferenced.cpp`](../Source/Syntax/SyntaxSymbol_NFACrossReferenced.cpp) | Recursively adds direct token edges for leading rule-call paths and attaches ordered return edges; original rule edges leave dispatch but remain the semantic source of packed `ReturnDesc` records. |
| [`Syntax/SyntaxSymbol_Automaton.cpp`](../Source/Syntax/SyntaxSymbol_Automaton.cpp) | Flattens the stable cross-referenced graph into `automaton::Executable` arrays, range slices, conditional text, instructions, returns, and metadata. |
| [`Syntax/SyntaxCppGen.h`](../Source/Syntax/SyntaxCppGen.h), [`Syntax/SyntaxCppGen.cpp`](../Source/Syntax/SyntaxCppGen.cpp) | Emit parser state enums, data loaders, rule/state labels, typed `@parser` entry points, and class-hierarchy callbacks. |

### Bootstrap syntax construction

| File | Responsibility |
| --- | --- |
| [`Syntax/SyntaxSymbol_CreateParserGenTypeSyntax.cpp`](../Source/Syntax/SyntaxSymbol_CreateParserGenTypeSyntax.cpp) | Builds the AST-definition grammar in the C++ DSL. |
| [`Syntax/SyntaxSymbol_CreateParserGenRuleSyntax.cpp`](../Source/Syntax/SyntaxSymbol_CreateParserGenRuleSyntax.cpp) | Builds the syntax-definition grammar, including condition precedence, EBNF, switches, clause kinds, and assignments. |
| [`Syntax/SyntaxSymbol_CreateParserGenUtility.cpp`](../Source/Syntax/SyntaxSymbol_CreateParserGenUtility.cpp) | Converts generated ParserGen token metadata into `syntax_writer::Token` values for those bootstrap grammars. |

## Trace-based recognition and AST scheduling

### Shared declarations and orchestration

| File | Responsibility |
| --- | --- |
| [`TraceManager/TraceManager.h`](../Source/TraceManager/TraceManager.h) | Declares typed handles, block pools, persistent return stacks, priority structures, traces, symbolic instruction records, ambiguity records, execution steps, manager state, and every phase method. |
| [`TraceManager/TraceManager.cpp`](../Source/TraceManager/TraceManager.cpp) | Implements survivor-buffer helpers, intrusive trace collection insertion/copying, pool accessors, construction, and `CreateExecutor`. |
| [`TraceManager/TraceManager_Common.h`](../Source/TraceManager/TraceManager_Common.h), [`TraceManager/TraceManager_Common.cpp`](../Source/TraceManager/TraceManager_Common.cpp) | Topological traversal of the surviving DAG and the virtual per-trace instruction view combining edge and popped-return slices. |

### Input and recognition

| File | Responsibility |
| --- | --- |
| [`TraceManager/TmInput.cpp`](../Source/TraceManager/TmInput.cpp) | Resets all state, drives one token round, filters final root configurations, builds successor links backward from success, and installs a direct step for an unambiguous chain. |
| [`TraceManager/TmInput_Walk.cpp`](../Source/TraceManager/TmInput_Walk.cpp) | Conditional-token checks, token transitions, return push/pop, qualified left-recursion transitions, lookahead-guided ending closure, and trace allocation. |
| [`TraceManager/TmInput_ReturnStack.cpp`](../Source/TraceManager/TmInput_ReturnStack.cpp) | Persistent return-stack successor cache and share-or-allocate logic. |
| [`TraceManager/TmInput_Competition.cpp`](../Source/TraceManager/TmInput_Competition.cpp) | Starts/inherits priority bets, closes them at rule endings or token boundaries, removes losing traces, and cleans active lists. |
| [`TraceManager/TmInput_Ambiguity.cpp`](../Source/TraceManager/TmInput_Ambiguity.cpp) | Defines equivalent reduction configurations, creates synthetic merge traces, and re-merges survivors after priority pruning. |

### Prepare-trace-route (`TmPtr`) phases

| File | Responsibility |
| --- | --- |
| [`TraceManager/TmPtr.cpp`](../Source/TraceManager/TmPtr.cpp) | Orders allocation, branch-shape construction, symbolic execution, and range summarization. |
| [`TraceManager/TmPtr_AllocateExecutionData.cpp`](../Source/TraceManager/TmPtr_AllocateExecutionData.cpp) | Allocates `TraceExec` and `InsExec` data in topological order and builds branch/merge linked lists. |
| [`TraceManager/TmPtr_BuildAmbiguityStructures.cpp`](../Source/TraceManager/TmPtr_BuildAmbiguityStructures.cpp) | Computes forward segment roots and the latest common forward branch at each merge. |
| [`TraceManager/TmPtr_PartialExecuteTraces.cpp`](../Source/TraceManager/TmPtr_PartialExecuteTraces.cpp) | Coordinates ordinary symbolic execution, merge compatibility checks, and merged contexts. |
| [`TraceManager/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.cpp`](../Source/TraceManager/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.cpp) | Interprets structural AST instructions into logical stack records and field/reuse dependencies without allocating AST nodes. |
| [`TraceManager/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.cpp`](../Source/TraceManager/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.cpp) | Verifies object/create stack depth and opening-depth agreement before a trace merge. |
| [`TraceManager/TmPtr_PartialExecuteTraces_MergeInsExecContext.cpp`](../Source/TraceManager/TmPtr_PartialExecuteTraces_MergeInsExecContext.cpp) | Aligns persistent symbolic stack levels and unions alternative logical stack IDs. |
| [`TraceManager/TmPtr_PartialExecuteTraces_SummarizeInstructionRange.cpp`](../Source/TraceManager/TmPtr_PartialExecuteTraces_SummarizeInstructionRange.cpp) | Propagates field/reuse dependencies to find each logical object's earliest begin, constructors, and possible bottom ends. |

### Resolve-ambiguity (`TmRa`) phases and replay

| File | Responsibility |
| --- | --- |
| [`TraceManager/TmRa.cpp`](../Source/TraceManager/TmRa.cpp) | Orders merge validation and execution-order construction. |
| [`TraceManager/TmRa_CheckMergeTraces.cpp`](../Source/TraceManager/TmRa_CheckMergeTraces.cpp) | Requires each trace merge to correspond to one local object ambiguity; derives boundaries, candidates, nesting, and critical-trace links. |
| [`TraceManager/TmRa_BuildExecutionOrder.cpp`](../Source/TraceManager/TmRa_BuildExecutionOrder.cpp) | Selects the candidates' common class, recursively expands nested ambiguity branches, builds step trees, duplicates per-candidate work, and linearizes the result. |
| [`TraceManager/TraceManager_ExecuteTrace.cpp`](../Source/TraceManager/TraceManager_ExecuteTrace.cpp) | Replays instruction ranges and synthetic ambiguity steps through `IAstInsReceiver`; removes empty `StackBegin`/`StackEnd` pairs. |

The `TmPtr`/`TmRa` naming is phase-based: prepare trace route first, resolve ambiguity second. Treat the header structures and all files in a phase as one algorithm.

## Checked-in generated parser-generator artifacts

[`ParserGen_Generated`](../Source/ParserGen_Generated) is the self-hosted output boundary. It contains the same artifact families the generator emits for users:

- `ParserGenTypeAst.*` and `ParserGenRuleAst.*`: typed definition AST classes.
- `*_Builder.*`, `*_Empty.*`, `*_Copy.*`, `*_Traverse.*`, `*_Json.*`, and `.d.ts`: optional AST utility families.
- `ParserGen_Lexer.*`: token enum, metadata, deleter, and embedded lexer data.
- `ParserGen_Assembler.*`: class/field IDs and concrete AST instruction receiver.
- `ParserGenTypeParser.*` and `ParserGenRuleParser.*`: executable parser data and typed definition parse entry points.

These files are generated through the staged workflow described in [Bootstrapping and verification](Bootstrapping.md). Edit their generator sources or definitions, then regenerate them through the required projects; do not patch them directly.

## Built-in JSON parser

| Files | Responsibility |
| --- | --- |
| [`Json/Syntax/Ast.txt`](../Source/Json/Syntax/Ast.txt), [`Lexer.txt`](../Source/Json/Syntax/Lexer.txt), [`Syntax.txt`](../Source/Json/Syntax/Syntax.txt) | Declarative JSON AST, tokens, and grammar. |
| [`Json/Generated`](../Source/Json/Generated) | Generated JSON AST, utilities, lexer, assembler, and parser. The JSON AST declarations are the DOM dependency imported by generated optional `_Json` utilities; core generated AST headers do not acquire this dependency. |
| [`Json/GlrJson.h`](../Source/Json/GlrJson.h), [`Json/GlrJson.cpp`](../Source/Json/GlrJson.cpp) | Stable wrapper: parsing with string/name unescaping, configurable printing, and node-list serialization. |

The wrapper owns domain semantics that should not be forced into the generic grammar runtime. In particular, generated token fields retain source spelling; `JsonUnescapeVisitor` converts quoted JSON strings into client values after parsing.

## Built-in XML parser

| Files | Responsibility |
| --- | --- |
| [`Xml/Syntax/Ast.txt`](../Source/Xml/Syntax/Ast.txt), [`Lexer.txt`](../Source/Xml/Syntax/Lexer.txt), [`Syntax.txt`](../Source/Xml/Syntax/Syntax.txt) | Declarative XML AST, tokens, and grammar. |
| [`Xml/Generated`](../Source/Xml/Generated) | Generated XML AST, utilities, lexer, assembler, and parser. |
| [`Xml/GlrXml.h`](../Source/Xml/GlrXml.h), [`Xml/GlrXml.cpp`](../Source/Xml/GlrXml.cpp) | Stable wrapper: parse/postprocess, escaping, printing, query helpers, list serialization, and fluent element construction. |

The XML wrapper tokenizes explicitly so its postprocessor can reconstruct whitespace gaps removed by the generic discarded-token filter, coalesce adjacent text nodes, and unescape attributes/CDATA/comments. Call the wrapper entry points rather than raw generated parser methods when these semantics matter.

## Current implementation notes

These observations describe the checked-in source. They are separated from the intended algorithms so a maintainer does not accidentally rely on a stale name or comment.

1. [`CompileSyntax_CalculateFirstSet.cpp`](../Source/ParserGen/CompileSyntax_CalculateFirstSet.cpp) does not currently persist FIRST sets. Prefix merge computes the real start-set closures later in [`SyntaxSymbol_NFACompact_PrefixMergeCrossReference.cpp`](../Source/Syntax/SyntaxSymbol_NFACompact_PrefixMergeCrossReference.cpp).
2. Switch test rewriting currently retains every satisfied branch and combines them as alternatives. This differs from the older syntax reference's first-satisfied-branch wording. See [`CompileSyntax_RewriteSyntax_Switch.cpp`](../Source/ParserGen/CompileSyntax_RewriteSyntax_Switch.cpp).
3. `AstDefFileGroup::dependencies` can be validated by `AddDependency`, but production source does not use the list for resolution or emission. `AstSymbolManager::fileMap` is declared but not populated. Treat both as dormant infrastructure.
4. `CppParserGenOutput::tokenIds` is populated during lexer header emission but is not consumed elsewhere in current source. Class and field ID maps are active cross-stage contracts.
5. `TraceManager::ComparePrefix` and `ComparePostfix` in [`TmRa_CheckMergeTraces.cpp`](../Source/TraceManager/TmRa_CheckMergeTraces.cpp) currently read both operands from the baseline instruction list. Their present effect is a length check, not the intended cross-branch instruction comparison.
6. The `InputIncomplete` branches in [`SyntaxBase.cpp`](../Source/SyntaxBase.cpp) currently appear reversed: the empty-token branch indexes the last token, while the nonempty branch returns a fixed zero range.
7. Some comments predate the current design: trace equality mentions runtime switch values even though switches are compiled away, and `Executable::astInstructions` mentions pre-input instructions even though current slices are post-input only.

Items 5 and 6 are behavior cautions, not alternative design rules. The main documents explain the invariants the surrounding algorithms are structured to enforce.

## Dependency-oriented reading guide

When diagnosing a generated parser problem, follow the representation rather than starting in the largest file:

1. Definition parse problem: generated Type/Rule parser → [`SyntaxBase`](../Source/SyntaxBase.h).
2. Semantic definition error: [`CompileAst`](../Source/ParserGen/CompileAst.cpp), [`CompileLexer`](../Source/ParserGen/CompileLexer.cpp), or the ordered `CompileSyntax_*` passes.
3. Wrong state graph: [`SyntaxSymbolWriter`](../Source/Syntax/SyntaxSymbolWriter.cpp) → compact-NFA files → cross-reference → automaton flattening.
4. Wrong accepted/rejected input: [`Executable`](../Source/Executable.h) → `TmInput_Walk` → return stacks/competitions/merge logic.
5. Correct recognition but wrong ambiguity: `TmPtr_*` → `TmRa_*`.
6. Correct instruction schedule but wrong field/object: [`AstBase`](../Source/AstBase.cpp) → generated assembler.
7. Correct generated AST but wrong JSON/XML value: handwritten [`GlrJson`](../Source/Json/GlrJson.cpp) or [`GlrXml`](../Source/Xml/GlrXml.cpp) postprocessing.

That sequence keeps each investigation at the narrowest layer that can own the failure.
