# Convert JSON back to generated ASTs

- The JSON switch generates both `json_(visitor|reader)::AstVisitor` in the same pair of files.
- Generated type would be `json_reader::AstVisitor`.
  - A member function `ReadJson`, taking a `JsonObject`, returning a `Ptr<ParsingAstBase>`.
  - In this function, it reads the type name, creates the AST type from the type name, and visits it to read the JSON object.
  - If a node has a base type, a dedicated base type function will be generated for sharing code.
  - `ReadJson` could be called recursively by each visitor, so it should push the JSON object so that the visitor knows where to read.
  - Just like in generated code how `Visit` calls `CopyFields` and `Traverse`, the `FillFields` functions will be actual functions that do the work.
  - So typically recursion happens like this: `ReadJson -> Visit -> FillFields -> ReadJson ...`
  - An incorrect type string, unknown field name, incompatible field type, etc. should throw. Any mistake in JSON should end with an exception. Missing fields do not count.
    - Any enum field should be initialized with the first item so that all fields have a default value.
- In `ParserTest_ParserGen_Generated` and built-in x4, verify `stringify(to_json) == stringify(to_json(from_json(to_json)))`.
  - `stringify`, `to_json`, and `from_json` are not real function names; they should be replaced with real things.
  - The left side of the equation should already have been logged in files, so there must be a variable that could be reused immediately.
- Both Debug x86 + x64 should be verified.
- In `ParserTest_LexerAndParser_Generated`, verify all kinds of `from_json` errors.
- Release VlppParser2 to Workflow and GacUI, release Workflow to GacUI, regenerate all parsers in Workflow and GacUI with updated GlrParserGen, pass their UnitTest projects.
  - Running `../Tools/Tools/Build.ps1 -Project VlppParser2|Workflow|GacUI` should perform these one after another.
  - To make sure the expected side effects (e.g. release, import, build tools, parser regeneration) are performed, after running each project, verify what changed via `git status`.

## DETAILS

- Extend the existing JSON optional-utility generator instead of adding another generated file family:
  - Keep generating `<Ast>_Json.h`, `<Ast>_Json.cpp`, and `<Ast>_Json.d.ts` from the `Json` switch. `<Json/>` in `BlockedUtilities` continues to suppress the whole family.
  - Keep the writer in `json_visitor` and generate the reader in the sibling `json_reader` namespace in the same `.h/.cpp` pair.
  - The generated class name is `<AstGroupName>Visitor`, matching the writer (for example, `ExprAstVisitor` for Calculator and `AstVisitor` for Workflow), rather than literally `AstVisitor` for every grammar.
  - Do not introduce a reflection dependency. Generate the type dispatch, enum dispatch, and field access directly from the AST definition.
- Generate this public entry point on the reader visitor:
  - `vl::Ptr<vl::glr::ParsingAstBase> ReadJson(vl::glr::json::JsonObject* json)`.
  - The `JsonObject*` is non-owning. A null argument throws an expected input exception.
  - Make the JSON DOM declarations available from the generated header and the complete JSON implementation available to the generated source without making unrelated AST consumers import the JSON parser implementation.
- The reader accepts the compact, range-free JSON contract emitted by `PrintAstJson` and described by the generated `.d.ts` file:
  - `$ast` is a required JSON string containing the same short concrete type name emitted by `json_visitor::WriteType`.
  - Token and enum fields are JSON strings.
  - AST object fields are a JSON object or JSON `null`.
  - AST array fields are JSON arrays whose elements are JSON objects or JSON `null`.
  - Range-enabled writer output and output generated with `printAstType = false` are not reader inputs. Deserialized AST and token code ranges, token index, and token type remain at their normal defaults.
- `ReadJson` performs the generated construction and visitor dispatch:
  - Resolve `$ast` to every instantiable generated AST class, including generated ambiguity-resolution classes, allocate that exact class, and reject missing, non-string, unknown, or abstract type names.
  - Push the current `JsonObject*`, visit the allocated node, and pop it on every return path, including exceptions. Nested AST fields recursively call `ReadJson`, so current-object state must be stack based and exception safe.
  - Generate one real `FillFields` function for each AST class. A concrete `Visit` calls `FillFields`; derived `FillFields` calls base `FillFields` first and then reads fields declared on the derived class.
  - Validate property names against the complete base-to-derived field set after filling, so a base `FillFields` does not reject a valid derived field. Treat `$ast` as a reserved consumed property.
- Convert fields as follows:
  - Copy a token from `JsonString::content.value`.
  - Match an enum's JSON string exactly to a declared enum item.
  - For an AST object, accept `null` as `nullptr`; otherwise recursively call `ReadJson` and require the result to cast to the field's declared AST type.
  - For an AST array, append to the new node's empty list, recursively read every non-null element, and require every result to cast to the declared element type. Preserve null elements.
- Missing ordinary fields are valid and keep deterministic defaults: empty token, null AST object, and empty AST array.
  - Before filling a newly allocated node, initialize each enum field to the first declared item. A present field overwrites that value. If an enum has no declared item, retain `UNDEFINED_ENUM_ITEM_VALUE`.
  - Keep the existing generated AST declaration initializer based on `UNDEFINED_ENUM_ITEM_VALUE`; parser assembly uses that sentinel for weak assignment. The first-item default belongs to JSON reading, not to general AST construction.
- Throw `vl::Exception` for malformed external JSON rather than an internal `CHECK_ERROR` failure. Reject at least:
  - Missing, duplicate, wrong-kind, unknown, or abstract `$ast` values.
  - Unknown or duplicate property names.
  - A wrong JSON kind for any token, enum, AST object, or AST array field.
  - An unknown enum item.
  - A recursively created AST type that is incompatible with the declared object or array-element type.
  - A non-object/non-null array element.
- Add the built-in JSON parser/shared-source dependency, or an equivalently narrow common test dependency, to every test project that parses the logged JSON before calling a reader. Use `vl::glr::json::JsonParse` so JSON strings are unescaped; do not call the generated JSON parser entry point directly.
- Regenerate generated files only through the ordered generator projects. Do not hand-edit generated AST, visitor, JSON, or TypeScript files.
- Update the affected documentation in the same change:
  - `doc/CodeGeneration.md` for the reader contract and generated JSON utility surface.
  - `doc/SourceMap.md` for source ownership/dependencies.
  - `.github/KnowledgeBase/KB_VlppParser2_Design_GlrParserGen.md` because GlrParserGen behavior changes.
  - `.github/KnowledgeBase/manual/vlppparser2/apis.md` for the generated API.

## VERIFICATION

- In `ParserTest_LexerAndParser_Generated`, exercise `json_reader::ExprAstVisitor` with syntactically valid compact JSON so failures come from the reader instead of the JSON parser:
  - Cover successful conversion of Calculator's token, enum, nullable AST object, AST array, standalone concrete class, and inherited fields.
  - Verify missing ordinary fields are accepted, including first-item defaults for missing enum fields.
  - Verify `null` is accepted for nullable AST fields and AST-array elements.
  - Cover every generated validation branch: null input; missing, duplicate, non-string, unknown, and abstract `$ast`; unknown and duplicate fields; wrong JSON kinds for each field category; unknown enum items; incompatible nested AST types; and invalid array elements.
- At every existing logged JSON site in `ParserTest_ParserGen_Generated`, `BuiltInTest_Json`, `BuiltInTest_Xml`, `BuiltInTest_Workflow`, and `BuiltInTest_Cpp`:
  - Reuse the existing `actualJson` or `astJson` variable.
  - Parse it with one shared `vl::glr::json::Parser` per test executable and `vl::glr::json::JsonParse`, require a `JsonObject`, read it with the matching generated `json_reader` visitor, and cast the result to the expected root AST type.
  - Serialize the restored AST with the same generated `json_visitor` and `PrintAstJson`, then assert exact string equality with the original variable. Existing log files and baselines must not change.
- From `Test/UnitTest`, run the complete ordered `Project.md` chain once for Debug/Win32 (the build-wrapper spelling of x86) and once for Debug/x64, using absolute paths to `.github/Scripts/copilotBuild.ps1` and `.github/Scripts/copilotExecute.ps1`:
  1. `ParserTest_AstGen`; build again afterward.
  2. `ParserTest_AstParserGen`; build again afterward.
  3. `ParserTest_LexerAndParser`; build again afterward.
  4. `ParserTest_LexerAndParser_Generated`.
  5. `ParserTest_ParserGen`.
  6. `ParserTest_ParserGen_Compiler`; build again afterward.
  7. `ParserTest_ParserGen_Generated`.
  8. `BuiltInTest_Compiler`; build again afterward.
  9. `BuiltInTest_Json`.
  10. `BuiltInTest_Xml`.
  11. `BuiltInTest_Workflow`.
  12. `BuiltInTest_Cpp`.
  - Inspect the overwritten `Build.log` or `Execute.log` immediately after every invocation. Require the documented success summary, no skipped modified test file, and no memory-leak dump.
- After all C++ tests, run `Test/TypeScript/prepare.ps1` by absolute path and then run `npm run build` from `Test/TypeScript`.
- From the VlppParser2 repository root, run the downstream release/import/regeneration commands separately and in this order; the release flow is additional Release verification and does not replace the complete Debug chain above:
  1. `& (Resolve-Path ../Tools/Tools/Build.ps1) -Project VlppParser2`
  2. `& (Resolve-Path ../Tools/Tools/Build.ps1) -Project Workflow`
  3. `& (Resolve-Path ../Tools/Tools/Build.ps1) -Project GacUI`
  - After each command, inspect its console output because `Build.ps1` catches failures, and inspect `git status --short` in VlppParser2, Workflow, GacUI, and Tools before continuing.
  - Confirm the expected release/import/build-tool/parser-regeneration changes, attribute every diff to the command that produced it, and reject unrelated changes before moving to the next project.
  - Confirm the downstream UnitTest projects pass for both architectures as driven by each project script.

## REVIEW COMMENTS
