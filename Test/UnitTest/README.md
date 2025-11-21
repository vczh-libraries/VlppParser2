# VlppParser2 Developer's Projects

Run the following projects in order.

- `ParserTest_AstGen`
  - Generate Calculator AST C++ types from manual definitions.
  - Generate Parser AST C++ types from manual definitions.
- `ParserTest_AstParserGen`
  - Run Calculator lexer from manual definitions.
  - Assembly to AST Building.
  - Generate Calculator lexer C++ types from manual definitions.
  - Generate Parser lexer C++ types from manual definitions.
- `ParserTest_LexerAndParser`
  - Generate Calculator parser C++ types from manual definitions.
  - Generate Parser parser C++ types from manual definitions.
- `ParserTest_LexerAndParser_Generated`
  - Run generated Calculator lexer types from previous projects.
  - Run Calculator parser from manual definitions.
- `ParserTest_ParserGen`
  - ParserGen error detection.
- `ParserTest_ParserGen_Compiler`
  - Run generated Calculator parser from previous projects.
  - Run generated Parser parser from previous projects.
    - Build multiple parsers from external syntax definitions.
- `ParserTest_ParserGen_Generated`
  - Run generated multiple parser from previous projects.

- `BuildInTest_Compiler`
  - Build real world parsers below
- `BuildInTest_Json`
  - Run generated JSON parser against real world examples.
- `BuildInTest_Xml`
  - Run generated XML parser against real world examples.
- `BuildInTest_Workflow`
  - Run generated Workflow parser against real world examples.
- `BuildInTest_Cpp`
  - Run generated C++ parser against real world examples.
