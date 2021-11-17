#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
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
	LogSyntax(
		syntaxManager,
		L"Calculator",
		L"NFA[1]",
		[](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); },
		[](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); },
		[](vint32_t token)
		{
			auto n = CalculatorTokenId((CalculatorTokens)token);
			auto d = CalculatorTokenDisplayText((CalculatorTokens)token);
			return d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n);
		});

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