# Convert JSON back to generated ASTs

- The json switch generates both `json_(visitor|reader)::AstVisitor` in the same pair of files.
- Generated type would be `json_reader::AstVisitor`.
  - A member function ReadJson, taking a JsonObject, returning a `Ptr<ParsingAstBase>`.
  - In this function, it reads the type name, create the ast type from the type name, visit it to read the json object.
  - If a node has base type, a dedicated base type function will be generated doe sharing code.
  - ReadJson could be called recursively by each visitor, so it should push the json object so that the visitor knows where to read.
  - Just like in generated code how `Visit` calls `CopyFields` and `Traverse`, the `FillFields` functions will be actual functions that do the work.
  - So typically recursion happens like this: ReadJson -> Visit -> FillFields -> ReadJson ...
  - Incorrect type string, unknown field name, incompatible field type, and etc, should throw, any mistake in JSON should ends up with an exception. But missing fields doesn't count.
    - Any enum field should be initialized with the first item so that all fields have a default value.
- In ParserTest_ParserGen_Generated and builtin x4, verify stringify(to_json) == stringify(to_json(from_json(to_jdon)))
  - stringify, to_json, from_json are not real function names, they should be replaced with real things.
  - the left side of the equation should already been logged in files so there must be a variable that could be reused immediately. 
- Both debug x86+x64 should be verified.
- In ParserGen_LexerAndParser_Generated, verify all kinds of from_json errors.
- Release VlppParser2 to Workflow and GacUI, release Workflow to GacUI, regenerate all parsers in Workflow and GacUI with updated GlrParserGen, pass their UnitTest projects.
  - Running `../Tools/Tools/Build.ps1 -Project VlppParser2|Workflow|GacUI` should perform this one after another.
  - But in order to make sure the expected side effects (e.g. release, import, build tools, parser regenerate) are performed, after running each project, you need to verify what is changed via `git status`.
