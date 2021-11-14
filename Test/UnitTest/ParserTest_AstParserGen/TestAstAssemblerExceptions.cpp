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
		WString input = LR"(
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
		WString input = LR"(
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
		WString input = LR"(
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
		WString input = LR"(
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
		WString input = LR"(
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

	TEST_CASE(L"InstructionNotComplete")
	{
		WString input = LR"(
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
		WString input = LR"(
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
		WString input = LR"(
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
		WString input = LR"(
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
		WString input = LR"(
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

	TEST_CASE(L"UnknownType")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::BeginObject, 0xFFFF }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnknownType); }
			);
	});

	TEST_CASE(L"UnknownField")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, 0xFFFF }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnknownField); }
			);
	});

	TEST_CASE(L"UnsupportedAmbiguityType")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::Expr, 2 }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnsupportedAmbiguityType); }
			);
	});

	TEST_CASE(L"FieldNotExistsInType")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::EnumItem, 0 }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Unary_op }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::FieldNotExistsInType); }
			);
	});

	TEST_CASE(L"FieldReassigned")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::FieldReassigned); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (TokenField = Enum)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::EnumItem, 0 }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (TokenField = Object)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (EnumField = Token)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Unary }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Unary_op }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (EnumField = Object)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Unary }, tokens[1]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Unary_op }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (ObjectField = Token)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Call }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (ObjectField = Enum)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Call }, tokens[1]);
		receiver.Execute({ AstInsType::EnumItem, 0 }, tokens[1]);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func }, tokens[1]),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

#undef LEXER
}