# VlppParser2

**General-LR Parser Generator (version 2)**

## License

This project is licensed under [the License repo](https://github.com/vczh-libraries/License).

Source code in this repo is for reference only, please use the source code in [the Release repo](https://github.com/vczh-libraries/Release).

You are welcome to contribute to this repo by opening pull requests.

## Document

(editing)

## Unit Test

For **Windows**, open `Test/UnitTest/UnitTest.sln`, and run the following projects in order:
- **ParserTest_AstGen**: Run AST related unit test and generate AST from the parser syntax.
- **ParserTest_AstParserGen**: Run AST instruction related unit test and generate lexer from the parser syntax.
- **ParserTest_LexerAndParser**: Run basic parsing unit test and generate parser from the parser syntax.
- **ParserTest_ParserGen**: Run unit test for detecting errors in parser syntax.
- **ParserTest_ParserGen_Compiler**: Generate many test only parsers for the following unit test, for testing different advanced features that the parser syntax offers.
- **ParserTest_ParserGen_Generated**: Run generated parsers and compare parser results with baselines.
- **BuiltInTest_Compiler**: Generate parser for built-in JSON, built-in XML and some test only parsers for the following unit tests:
  - **BuiltInTest_Json**
  - **BuiltInTest_Xml**
  - **BuiltInTest_Workflow**
  - **BuiltInTest_Cpp**

For **Linux**, use `Test/Linux/*/makefile` to build and run unit test projects as described above.
