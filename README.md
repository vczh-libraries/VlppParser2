# VlppParser2

**General-LR Parser Generator (version 2)**

## License

This project is licensed under [the License repo](https://github.com/vczh-libraries/License).

Source code in this repo is for reference only, please use the source code in [the Release repo](https://github.com/vczh-libraries/Release).

You are welcome to contribute to this repo by opening pull requests.

## Document

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/vczh-libraries/VlppParser2)

(editing)

## Unit Test

### Windows

Build `Test/UnitTest/UnitTest.sln`, then run these projects in order. Rebuild the solution at every explicit rebuild barrier because the preceding project generates C++ consumed by later projects.

1. **ParserTest_AstGen**: Test AST support and generate Calculator and parser-definition AST types.
2. **Rebuild the solution.**
3. **ParserTest_AstParserGen**: Test the manual Calculator lexer and AST assembler, then generate Calculator and parser-definition lexer types.
4. **Rebuild the solution.**
5. **ParserTest_LexerAndParser**: Test basic parsing and generate Calculator and parser-definition parser types.
6. **Rebuild the solution.**
7. **ParserTest_LexerAndParser_Generated**: Test the generated Calculator lexer and the manually defined Calculator parser.
8. **ParserTest_ParserGen**: Test parser-definition error detection.
9. **ParserTest_ParserGen_Compiler**: Test the generated Calculator and parser-definition parsers, then generate the external parsers used by the next project.
10. **Rebuild the solution.**
11. **ParserTest_ParserGen_Generated**: Run the generated external parsers and compare their results with baselines.
12. **BuiltInTest_Compiler**: Generate the JSON, XML, Workflow, and C++ parsers.
13. **Rebuild the solution.**
14. Run **BuiltInTest_Json**, **BuiltInTest_Xml**, **BuiltInTest_Workflow**, and **BuiltInTest_Cpp**, in that order.

`BuiltInTest_Workflow` expects the [Workflow repository](https://github.com/vczh-libraries/Workflow) to be cloned as a sibling of this repository.

The repository-provided command-line wrappers can be run from `Test/UnitTest` as follows. The build wrapper requires `VLPP_VSDEVCMD_PATH` to point to Visual Studio's `VsDevCmd.bat`.

```powershell
cd Test\UnitTest
& (Resolve-Path ..\..\.github\Scripts\copilotBuild.ps1)
& (Resolve-Path ..\..\.github\Scripts\copilotExecute.ps1) -Mode UnitTest -Executable PROJECT-NAME
```

After all native projects pass, use Node.js and npm to verify the generated TypeScript declarations. From the repository root:

```powershell
cd Test\TypeScript
npm ci
& (Resolve-Path .\prepare.ps1)
npm run build
```

`npm ci` is needed on a fresh checkout or after the lock file changes.

### Linux

Linux currently has configurations for **ParserTest_ParserGen_Compiler**, **ParserTest_ParserGen_Generated**, **BuiltInTest_Json**, **BuiltInTest_Xml**, and **BuiltInTest_Workflow**. It does not contain the full Windows bootstrap sequence or BuiltIn-Cpp test. Run the two ParserGen projects in that order; the built-in parser projects can then be verified individually.

From each `Test/Linux/PROJECT-NAME` directory, use the repository build wrapper and then run the produced unit test:

```bash
../../../.github/Ubuntu/build.sh
./Bin/UnitTest /C
```

Do not invoke the generated `makefile` directly. Only Debug x64 is supported. The Workflow sibling-repository prerequisite also applies to **BuiltInTest_Workflow**.
