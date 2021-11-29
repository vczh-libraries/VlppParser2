#include "../../../Source/ParserGen_Generated/ParserGenTypeAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../Source/LogAutomaton.h"

using namespace vl::glr::parsergen;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	TypeParser typeParser;

	TEST_CATEGORY(L"Test ParserGen on Calculator")
	{
		FilePath dirParser = GetTestParserInputPath(L"Calculator");
		FilePath dirOutput = GetOutputDir(L"ParserGen");

		TEST_CASE(L"Ast.txt")
		{
			auto astCode = File(dirParser / L"Syntax/Ast.txt").ReadAllTextByBom();
			auto ast = typeParser.ParseFile(astCode);
			auto actualJson = PrintAstJson<json_visitor::TypeAstVisitor>(ast);
			File(dirOutput / L"Output[Calculator].txt").WriteAllText(actualJson, BomEncoder::Utf8);
		});
	});

#undef LEXER
}