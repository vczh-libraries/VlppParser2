#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;
using namespace vl::glr;
using namespace vl::glr::parsergen;
using namespace calculator;

extern WString GetExePath();
extern void GenerateCalculatorLexer(LexerSymbolManager& manager);

TEST_FILE
{
	ParserSymbolManager global;
	LexerSymbolManager lexerManager(global);
	GenerateCalculatorLexer(lexerManager);

	RegexLexer lexer(
		From(lexerManager.TokenOrder())
			.Select([&](const WString& name) { return lexerManager.Tokens()[name]->regex; })
		);
	TEST_CASE_ASSERT(lexerManager.TokenOrder().IndexOf(L"SPACE") == 23);

#define LEXER(INPUT, NAME)\
		List<RegexToken> NAME;\
		lexer.Parse(INPUT).ReadToEnd(NAME, [](vint id) { return id == 23; })\

	TEST_CASE(L"NoRootObject")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Token }, tokens[0]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoRootObject); }
			);
	});

#undef LEXER
}