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

/***********************************************************************
Common Exceptions
***********************************************************************/

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

	TEST_CASE(L"MissingFieldValue")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::MissingFieldValue); }
			);
	});

	TEST_CASE(L"MissingAmbiguityCandidate")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, -1, 2 }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::MissingAmbiguityCandidate); }
			);
	});

	TEST_CASE(L"AmbiguityCandidateIsNotObject")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, -1, 2 }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::AmbiguityCandidateIsNotObject); }
			);
	});

	TEST_CASE(L"AmbiguityCandidateIsNotObject")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::EnumItem, 0 }, tokens[1]);
		receiver.Execute({ AstInsType::EnumItem, 1 }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, -1, 2 }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::AmbiguityCandidateIsNotObject); }
			);
	});

	TEST_CASE(L"UnassignedObjectLeaving")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::EndObject }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnassignedObjectLeaving); }
			);
	});

	TEST_CASE(L"InstructionNotComplete")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		TEST_EXCEPTION(
			receiver.Finished(),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::InstructionNotComplete); }
			);
	});

	TEST_CASE(L"Corrupted (1)")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		try { receiver.Finished(); } catch (...) {}
		TEST_EXCEPTION(
			receiver.Finished(),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::Corrupted); }
			);
	});

	TEST_CASE(L"Corrupted (2)")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		try { receiver.Finished(); } catch (...) {}
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::EndObject }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::Corrupted); }
			);
	});

	TEST_CASE(L"Finished (1)")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		receiver.Finished();
		TEST_EXCEPTION(
			receiver.Finished(),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::Finished); }
			);
	});

	TEST_CASE(L"Finished (2)")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		receiver.Finished();
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::EndObject }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::Finished); }
			);
	});

/***********************************************************************
Generated AST Exceptions
***********************************************************************/

#undef LEXER
}