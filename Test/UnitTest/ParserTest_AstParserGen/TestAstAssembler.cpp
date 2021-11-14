#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/CalculatorAst_Json.h"

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

	TEST_CASE(L"export 1")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
	});

#undef LEXER
}