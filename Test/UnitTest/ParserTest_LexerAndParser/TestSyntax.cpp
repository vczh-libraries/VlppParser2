#include "../../Source/Calculator/Parser/CalculatorAst.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../Source/LogParser.h"

using namespace vl::regex;
using namespace calculator;

extern void GenerateCalculatorSyntax(SyntaxSymbolManager& manager);

TEST_FILE
{
	ParserSymbolManager global;
	SyntaxSymbolManager syntaxManager(global);
	GenerateCalculatorSyntax(syntaxManager);
	TEST_CASE_ASSERT(global.Errors().Count() == 0);
	LogSyntax(syntaxManager, L"Calculator", L"NFA[1]");

	MemoryStream lexerData;
	CalculatorLexerData(lexerData);
	lexerData.SeekFromBegin(0);
	RegexLexer lexer(lexerData);

	TEST_CASE(L"Launch Calculator Syntax")
	{
		WString input = LR"(
export 1
)";
		List<RegexToken> tokens;
		lexer.Parse(input).ReadToEnd(tokens, CalculatorTokenDeleter);
	});

#undef LEXER
}