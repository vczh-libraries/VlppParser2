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

Here is a list of unit test projects in `REPO-ROOT/Test/UnitTest/{NAME}/{NAME}.vcxproj` folder, you are required to run all of them in order:
- `ParserTest_AstGen`: Generate Calculator AST C++ types from manual definitions. Generate Parser AST C++ types from manual definitions.
- `ParserTest_AstParserGen`: Run Calculator lexer from manual definitions. Assembly to AST Building. Generate Calculator lexer C++ types from manual definitions. Generate Parser lexer C++ types from manual definitions.
- `ParserTest_LexerAndParser`: Generate Calculator parser C++ types from manual definitions. Generate Parser parser C++ types from manual definitions.
- `ParserTest_LexerAndParser_Generated`: Run generated Calculator lexer types from previous projects. Run Calculator parser from manual definitions.
- `ParserTest_ParserGen`: ParserGen error detection.
- `ParserTest_ParserGen_Compiler`: Run generated Calculator parser from previous projects. Run generated Parser parser from previous projects. Build multiple parsers from external syntax definitions.
- `ParserTest_ParserGen_Generated`: Run generated multiple parsers from previous projects.
- `BuiltInTest_Compiler`: Build real world parsers below.
- `BuiltInTest_Json`: Run generated JSON parser against real world examples.
- `BuiltInTest_Xml`: Run generated XML parser against real world examples.
- `BuiltInTest_Workflow`: Run generated Workflow parser against real world examples.
- `BuiltInTest_Cpp`: Run generated C++ parser against real world examples.
When any *.h or *.cpp file is changed, unit test is required to run.

When any test case fails, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.
