# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

## Task 1: New C++ function to create .d.ts files for AST defined in a parser.

You are going to complete a new function in `AstCppGen_JsonVisitor.cpp`.
- Add `jsonDts` to `CppAstGenOutput` class, file names would be the same to `jsonH` and `jsonCpp` but with the `.d.ts` extension name.
- Add `WriteJsonVisitorDtsFile` to `AstCppGen_JsonVisitor.cpp`
- Connect them in `WriteAstFiles` function in `AstCppGen.cpp`
- Add `jsonDts` to the "Json" list in `Tools\GlrParserGen\GlrParserGen\Main.cpp`

The `WriteJsonVisitorDtsFile` function is going to create a .d.ts file that has the typescript schema for printed JSON from generated AST. The .d.ts file should define TypeScript types matching the JSON output of the AST JSON visitor.

## Task 2. Verify generated .d.ts files

Create a `Test\TypeScript` folder as a nodejs package that verifies generated .d.ts files by type-checking JSON output against the TypeScript schemas. Use `npm install` and `npm run build` to validate.

# UPDATES

# TEST [CONFIRMED]

The test is two-fold:
1. **C++ test**: Run all unit test projects in order. The `BuiltInTest_Compiler` test will generate `.d.ts` files for Xml/Json/Workflow parsers. All test cases must pass without crash.
2. **TypeScript test**: In `Test\TypeScript`, run `npm install` and `npm run build`. All `.ts` files must compile successfully, validating that generated JSON matches the `.d.ts` schemas.

Success criteria:
- All C++ unit test projects pass.
- `npm run build` in `Test\TypeScript` succeeds with zero errors.
- Generated `.d.ts` files exist at expected locations.

# PROPOSALS

- No.1 Implement WriteJsonVisitorDtsFile and TypeScript verification project [CONFIRMED]

## No.1 Implement WriteJsonVisitorDtsFile and TypeScript verification project

Implement the full solution:
1. Add `jsonDts` field to `CppAstGenOutput` struct
2. Implement `WriteJsonVisitorDtsFile` function that generates TypeScript type definitions from AST symbols
3. Wire it into `WriteAstFiles` and `GenerateAstFileNames`
4. Add `jsonDts` to the Json blocked utilities list in `Main.cpp`
5. Create `Test\TypeScript` nodejs package for verification

### CODE CHANGE

**Source/ParserGen_Global/ParserCppGen.h**: Added `WString jsonDts;` field to `CppAstGenOutput` struct.

**Source/Ast/AstCppGen.h**: Added declaration `extern void WriteJsonVisitorDtsFile(...)`.

**Source/Ast/AstCppGen.cpp**:
- In `GenerateAstFileNames`: Added `astOutput->jsonDts = globalName + group->Name() + L"_Json.d.ts";`
- In `WriteAstFiles`: Added block to generate .d.ts file via `WriteJsonVisitorDtsFile`.

**Source/Ast/AstCppGen_JsonVisitor.cpp**: Added helper functions and `WriteJsonVisitorDtsFile`:
- `CollectAllLeafClasses`: Recursively collects all leaf classes from a class hierarchy.
- `FindNearestAncestorWithProps`: Finds nearest ancestor with properties for `extends` chain.
- `WriteDtsPropType`: Converts AST property types to TypeScript types (Token→string, Enum→EnumName, Class→ClassName | null, Array→(ClassName | null)[]).
- `WriteJsonVisitorDtsFile`: Main function that generates:
  - Enum types as `export type EnumName = "Item1" | "Item2" | ...;`
  - Union types for abstract classes (all leaf classes recursively collected)
  - `_Common` interfaces for abstract classes with properties
  - Concrete interfaces with `$ast` discriminant field, inheriting `_Common` where needed

**Tools/GlrParserGen/GlrParserGen/Main.cpp**: Added `files.Remove(astOutput->jsonDts);` to the "Json" blocked utilities section.

**Test/TypeScript/**: Created complete nodejs package:
- `.gitignore`: Ignores node_modules/, Xml/, Json/, Workflow/ (generated)
- `package.json`: devDependency on typescript ^5.8.3
- `tsconfig.json`: strict mode, noEmit
- `prepare.ps1`: Script that copies .d.ts files and generates .ts test files from JSON outputs

### CONFIRMED

All C++ unit tests pass (ParserTest_AstGen through BuiltInTest_Cpp, 12 test projects).
TypeScript verification (`npm run build`) exits with code 0, confirming that all 664+ JSON test outputs match the generated .d.ts schemas.
Generated .d.ts files exist at:
- Source/Xml/Generated/XmlAst_Json.d.ts
- Source/Json/Generated/JsonAst_Json.d.ts
- Test/Source/BuiltIn-Workflow/Generated/WorkflowAst_Json.d.ts
