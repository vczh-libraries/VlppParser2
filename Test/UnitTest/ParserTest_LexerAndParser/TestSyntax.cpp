#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../Source/LogParser.h"

using namespace vl::regex;
using namespace calculator;

extern void GenerateCalculatorSyntax(SyntaxSymbolManager& manager);

namespace TestSyntax_TestObjects
{
	void LogCalculatorSyntax(SyntaxSymbolManager& syntaxManager, const WString& phase)
	{
		LogSyntax(
			syntaxManager,
			L"Calculator",
			phase,
			[](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); },
			[](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); },
			[](vint32_t token)
			{
				auto n = CalculatorTokenId((CalculatorTokens)token);
				auto d = CalculatorTokenDisplayText((CalculatorTokens)token);
				return d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n);
			});
	}
}
using namespace TestSyntax_TestObjects;

TEST_FILE
{
	ParserSymbolManager global;
	SyntaxSymbolManager syntaxManager(global);
	GenerateCalculatorSyntax(syntaxManager);
	TEST_CASE_ASSERT(global.Errors().Count() == 0);
	LogCalculatorSyntax(syntaxManager, L"NFA[1]");

	syntaxManager.BuildCompactSyntax();
	TEST_CASE_ASSERT(global.Errors().Count() == 0);
	LogCalculatorSyntax(syntaxManager, L"NFA[2]");

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