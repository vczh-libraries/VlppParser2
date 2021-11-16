#include "../../Source/Calculator/Parser/CalculatorAst.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../../Source/Syntax/SyntaxSymbol.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex;
using namespace calculator;
using namespace vl::glr::parsergen;

extern WString GetExePath();
extern void GenerateCalculatorSyntax(SyntaxSymbolManager& manager);

TEST_FILE
{
	ParserSymbolManager global;
	SyntaxSymbolManager syntaxManager(global);
	GenerateCalculatorSyntax(syntaxManager);
	TEST_CASE_ASSERT(global.Errors().Count() == 0);

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