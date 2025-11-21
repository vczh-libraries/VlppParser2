#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;
using namespace vl::glr;
using namespace vl::glr::parsergen;
using namespace calculator;

extern void GenerateCalculatorLexer(LexerSymbolManager& manager);

namespace
{
	void BuildNumExprToSlot(CalculatorAstInsReceiver& receiver, List<RegexToken>& tokens, vint32_t tokenIndex, vint slotIndex)
	{
		receiver.Execute({ AstInsType::StackBegin }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value, 0 }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::StackEnd }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::StackSlot, -1, slotIndex }, tokens[tokenIndex], tokenIndex);
	}

	void BuildTrueToSlot(CalculatorAstInsReceiver& receiver, List<RegexToken>& tokens, vint32_t tokenIndex, vint slotIndex)
	{
		receiver.Execute({ AstInsType::StackBegin }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::True }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::StackEnd }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::StackSlot, -1, slotIndex }, tokens[tokenIndex], tokenIndex);
	}

	void BuildMinimalModule(CalculatorAstInsReceiver& receiver, List<RegexToken>& tokens)
	{
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		BuildNumExprToSlot(receiver, tokens, 1, 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::StackEnd }, tokens[1], 1);
	}
}

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
		lexer.Parse(INPUT).ReadToEnd(NAME, [](vint id) { return id == 23; })

/***********************************************************************
Common Exceptions
***********************************************************************/

		TEST_CASE(L"NoCreatingObjectForStackField")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, 0xFFFF, 0 }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoCreatingObjectForField); }
		);
	});

	TEST_CASE(L"NoStackFrameForToken")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoStackFrame); }
		);
	});

	TEST_CASE(L"NoCreatingObjectForStackSlot")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoCreatingObjectForStackSlot); }
			);
	});

	TEST_CASE(L"NoCreatingObjectForStackEnd")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::StackEnd }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoCreatingObjectForStackEnd); }
			);
	});

	TEST_CASE(L"NoStackFrameForStackEnd")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::StackEnd }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoStackFrameForStackEnd); }
			);
	});

	TEST_CASE(L"NoStackFrameForField")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 0 }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoStackFrame); }
		);
	});

	TEST_CASE(L"NoStackFrameForResolveAmbiguity")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::Expr, 0 }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::NoStackFrame); }
		);
	});

	TEST_CASE(L"CreatingObjectNotReset (CreateObject)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::CreatingObjectNotReset); }
			);
	});

	TEST_CASE(L"CreatingObjectNotReset (ResolveAmbiguity)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::NumExpr }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::CreatingObjectNotReset); }
		);
	});

	TEST_CASE(L"InstructionNotComplete")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		try { receiver.Finished(); } catch (...) {}
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::StackEnd }, tokens[1], 1),
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
		BuildMinimalModule(receiver, tokens);
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
		BuildMinimalModule(receiver, tokens);
		receiver.Finished();
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::CreateObject, 0xFFFF }, tokens[0], 0),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, 0xFFFF, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnknownField); }
			);
	});

	TEST_CASE(L"UnsupportedAbstractType")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Expr }, tokens[0], 0),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnsupportedAbstractType); }
			);
	});

	TEST_CASE(L"FieldNotExistsInType")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::EnumItem, 0, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Unary_op, 0 }, tokens[1], 1),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value, 0 }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::FieldReassigned); }
			);
	});

	TEST_CASE(L"FieldWeakAssignmentOnNonEnum")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		BuildNumExprToSlot(receiver, tokens, 1, 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Module_exported, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::FieldWeakAssignmentOnNonEnum); }
			);
	});

	TEST_CASE(L"ObjectTypeMismatchedToField (TokenField = Enum)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::EnumItem, 0, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value, 0 }, tokens[1], 1),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		BuildTrueToSlot(receiver, tokens, 1, 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value, 0 }, tokens[1], 1),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Unary }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Unary_op, 0 }, tokens[1], 1),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		BuildTrueToSlot(receiver, tokens, 1, 0);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Unary }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Unary_op, 0 }, tokens[1], 1),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Call }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func, 0 }, tokens[1], 1),
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
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::EnumItem, 0, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Call }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::ObjectTypeMismatchedToField); }
			);
	});

/***********************************************************************
Generated AST Exceptions (Ambiguity)
***********************************************************************/

	TEST_CASE(L"UnsupportedAmbiguityType")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		BuildNumExprToSlot(receiver, tokens, 1, 0);
		BuildNumExprToSlot(receiver, tokens, 1, 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::NumExpr, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::UnsupportedAmbiguityType); }
			);
	});

	TEST_CASE(L"UnexpectedAmbiguousCandidate")
	{
		TEST_ASSERT(true);
	});

	TEST_CASE(L"MissingAmbiguityCandidate (0)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::Expr, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::MissingAmbiguityCandidate); }
		);
	});

	TEST_CASE(L"MissingAmbiguityCandidate (1)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		BuildNumExprToSlot(receiver, tokens, 1, 0);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::Expr, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::MissingAmbiguityCandidate); }
			);
	});

	TEST_CASE(L"AmbiguityCandidateIsNotObject (Token)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::Expr, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::AmbiguityCandidateIsNotObject); }
			);
	});

	TEST_CASE(L"AmbiguityCandidateIsNotObject (EnumItem)")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
		receiver.Execute({ AstInsType::EnumItem, 0, 0 }, tokens[1], 1);
		receiver.Execute({ AstInsType::EnumItem, 1, 0 }, tokens[1], 1);
		TEST_EXCEPTION(
			receiver.Execute({ AstInsType::ResolveAmbiguity, (vint32_t)CalculatorClasses::Expr, 0 }, tokens[1], 1),
			AstInsException,
			[](const AstInsException& e) { TEST_ASSERT(e.error == AstInsErrorType::AmbiguityCandidateIsNotObject); }
			);
	});

#undef LEXER
}
