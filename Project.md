# Documentation entry points

- Begin with [`doc/Index.md`](./doc/Index.md) for the architecture, parser-generation pipeline, runtime parsing and ambiguity algorithms, generated-code contracts, source map, and bootstrap guidance.
- Begin with [`doc/cpp/Index.md`](./doc/cpp/Index.md) for BuiltIn-Cpp syntax design, C++26 parser capability, frontend boundaries, standard-coverage gaps, and de-ambiguation guidance.
- Before making a BuiltIn-Cpp syntax or coverage decision, read [`doc/cpp/Philosophy.md`](./doc/cpp/Philosophy.md). It defines the code-indexing goal and the priority order for historical compatibility, orthogonality, bounded over-acceptance, and avoiding unnecessary ambiguity.

Treat these documents as semantic companions to the code. When a related code change alters a documented behavior, invariant, public grammar contract, or responsibility boundary, update the owning document in the same change. Do not churn documentation for formatting, generated output, or mechanical code refactoring that leaves those semantics unchanged; reference repairs are still required when files move.

# General Instruction

## Solution to Work On

You are working on the solution `REPO-ROOT/Test/UnitTest/UnitTest.sln`,
therefore `SOLUTION-ROOT` is `REPO-ROOT/Test/UnitTest`.

## Files not Allowed to Modify

Files in these folders (recursively) are not allowed to modify.
You can only change them using what is described in the `Projects for Verification` section.
If you encounter any error that prevent these files from being generated,
always fix the root cause.
- `REPO-ROOT/Source/ParserGen_Generated`
- `REPO-ROOT/Source/Json/Generated`
- `REPO-ROOT/Source/Xml/Generated`

Files in `REPO-ROOT/Import` and `REPO-ROOT/Release` (recursively) are also not allowed to modify.
These files are prepared for foreign dependencies.

## Projects for Verification

Here is a list of unit test projects in `REPO-ROOT/Test/UnitTest/{NAME}/{NAME}.vcxproj` folder, you are required to run all of them in order.
Each project must be executed in the correct order because most of them generate files for following projects.
- `ParserTest_AstGen`: Generate Calculator AST C++ types from manual definitions. Generate Parser AST C++ types from manual definitions.
  - **Rebuild the solution after execution**.
- `ParserTest_AstParserGen`: Run Calculator lexer from manual definitions. Assembly to AST Building. Generate Calculator lexer C++ types from manual definitions. Generate Parser lexer C++ types from manual definitions.
  - **Rebuild the solution after execution**.
- `ParserTest_LexerAndParser`: Generate Calculator parser C++ types from manual definitions. Generate Parser parser C++ types from manual definitions.
  - **Rebuild the solution after execution**.
- `ParserTest_LexerAndParser_Generated`: Run generated Calculator lexer types from previous projects. Run Calculator parser from manual definitions.
- `ParserTest_ParserGen`: ParserGen error detection.
- `ParserTest_ParserGen_Compiler`: Run generated Calculator parser from previous projects. Run generated Parser parser from previous projects. Build multiple parsers from external syntax definitions.
  - **Rebuild the solution after execution**.
- `ParserTest_ParserGen_Generated`: Run generated multiple parsers from previous projects.
- `BuiltInTest_Compiler`: Build real world parsers below.
  - **Rebuild the solution after execution**.
- `BuiltInTest_Json`: Run generated JSON parser against real world examples.
- `BuiltInTest_Xml`: Run generated XML parser against real world examples.
- `BuiltInTest_Workflow`: Run generated Workflow parser against real world examples from `Workflow` repo directly. It assumes `Workflow` repo is cloned as a sibiling folder.
- `BuiltInTest_Cpp`: Run generated C++ parser against real world examples.

In `REPO-ROOT/Test/TypeScript` there is a TypeScript package, it will becomes available after running `BuiltInTest_(Compiler|Json|Xml|Workflow)`. You need to run `prepare.ps1` followed by `npm run build` and ensure you don't see any error. This project verifies if JSON schema of ASTs is properly generated.

When any *.h or *.cpp file is changed, unit test is required to run.
When any test case fails, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.

## Verifying Generated .d.ts Files

All C++ unit test projects have to be successfully completed first.
Run `REPO-ROOT/Test/TypeScript/prepare.ps1`, and it copies all generated .d.ts files to this folder.
Generated .d.ts files are JSON representation of AST, creating from the parser with inputs.
All inputs will also be copied here, becoming .ts files matching ASTs with AST type definitions.
Build `REPO-ROOT/Test/TypeScript/package/json` to make sure all generated .d.ts files are valid.

## Linux Specific

`REPO-ROOT/Test/Linux` stores linux configurations for:
- `BuiltInTest_Json`: `BuiltInTest_Json.vcxproj`.
- `BuiltInTest_Xml`: `BuiltInTest_Xml.vcxproj`.
- `BuiltInTest_Workflow`: `BuiltInTest_Workflow.vcxproj`.
- `ParserTest_ParserGen_Compiler`: `ParserTest_ParserGen_Compiler.vcxproj`, `ParserTest_ParserGen.vcxproj`.
- `ParserTest_ParserGen_Generated`: `ParserTest_ParserGen_Generated.vcxproj`.

There are many MSBuild test projects that are not included in this folder.
Linux supports only the five configurations listed above and does not provide the full Windows bootstrap and generator sequence.

You need to build, test and debug in that specific folder, otherwise the unit test will not function properly.
On Linux, only configuration "debug x64" is available, no need to build or run projects with other configurations.
Unlike Windows, building have to be done in each folder separately.
