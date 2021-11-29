#include "../../../Source/ParserGen_Generated/ParserGenTypeAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../Source/LogAutomaton.h"

using namespace vl::glr::parsergen;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	TEST_CATEGORY(L"Test ParserGen on Calculator")
	{
		FilePath dirParser = GetTestParserInputPath(L"Calculator");
		FilePath dirOutput = GetOutputDir(L"ParserGen");

		TEST_CASE(L"Ast.txt")
		{
			auto input = File(dirParser / L"Syntax/Ast.txt").ReadAllTextByBom();
			auto ast = typeParser.ParseFile(input);
			auto actualJson = PrintAstJson<json_visitor::TypeAstVisitor>(ast);
			File(dirOutput / L"Ast[Calculator].txt").WriteAllText(actualJson, BomEncoder::Utf8);
		});

		TEST_CASE(L"Syntax.txt")
		{
			auto input = File(dirParser / L"Syntax/Syntax.txt").ReadAllTextByBom();
			auto ast = ruleParser.ParseFile(input);
			auto actualJson = PrintAstJson<json_visitor::RuleAstVisitor>(ast);
			File(dirOutput / L"Syntax[Calculator].txt").WriteAllText(actualJson, BomEncoder::Utf8);
		});
	});

#undef LEXER
}