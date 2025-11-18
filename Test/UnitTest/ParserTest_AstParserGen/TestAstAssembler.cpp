#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../Source/Calculator/Parser/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/LogParser.h"

using namespace vl::glr::parsergen;
using namespace calculator;

extern void GenerateCalculatorLexer(LexerSymbolManager& manager);

namespace
{
	void BuildNumExpr(CalculatorAstInsReceiver& receiver, List<RegexToken>& tokens, vint tokenIndex)
	{
		receiver.Execute({ AstInsType::StackBegin }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[tokenIndex + 1], tokenIndex + 1);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value, 0 }, tokens[tokenIndex + 1], tokenIndex + 1);
		receiver.Execute({ AstInsType::StackEnd }, tokens[tokenIndex + 1], tokenIndex + 1);
	}

	void BuildImport(CalculatorAstInsReceiver& receiver, List<RegexToken>& tokens, vint tokenIndex)
	{
		receiver.Execute({ AstInsType::StackBegin }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Import }, tokens[tokenIndex + 1], tokenIndex + 1);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Import_name, 0 }, tokens[tokenIndex + 1], tokenIndex + 1);
		receiver.Execute({ AstInsType::StackEnd }, tokens[tokenIndex + 1], tokenIndex + 1);
	}

	void BuildRef(CalculatorAstInsReceiver& receiver, List<RegexToken>& tokens, vint tokenIndex)
	{
		receiver.Execute({ AstInsType::StackBegin }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[tokenIndex], tokenIndex);
		receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Ref }, tokens[tokenIndex + 1], tokenIndex + 1);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Ref_name, 0 }, tokens[tokenIndex + 1], tokenIndex + 1);
		receiver.Execute({ AstInsType::StackEnd }, tokens[tokenIndex + 1], tokenIndex + 1);
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
		lexer.Parse(INPUT).ReadToEnd(NAME, [](vint id) { return id == 23; })\

	TEST_CASE(L"export 1")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 2);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[1], 1);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[1], 1);
			receiver.Execute({ AstInsType::StackEnd }, tokens[1], 1);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "NumExpr",
        "value": "1"
    },
    "imports": []
})");
	});

	TEST_CASE(L"export (1) <ReopenObject>")
	{
		WString input = LR"(
export (1)
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0); // export
			receiver.Execute({ AstInsType::StackBegin }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[1], 1); // (
			BuildNumExpr(receiver, tokens, 2); // 1
			receiver.Execute({ AstInsType::Token, -1, 2 }, tokens[3], 3); // )
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "NumExpr",
        "value": "1"
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			receiver.Execute({ AstInsType::StackBegin }, tokens[1], 1);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2 <FieldIfUnassigned>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			receiver.Execute({ AstInsType::StackBegin }, tokens[1], 1);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2 <FieldIfUnassigned (canceled)>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			receiver.Execute({ AstInsType::StackBegin }, tokens[1], 1);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Multiply, 4 }, tokens[3], 3);
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op, 4 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2 <LeftRecursion + FieldIfUnassigned>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackBegin }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2 <LeftRecursion + FieldIfUnassigned (canceled)>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackBegin }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Multiply, 4 }, tokens[3], 3);
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op, 4 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2 <LeftRecursion>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackBegin }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"export 1 + 2 <LeftRecursion injection>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0);
			BuildNumExpr(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackBegin }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackBegin }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackBegin }, tokens[3], 3);
			BuildNumExpr(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem, (vint32_t)BinaryOp::Add, 3 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Binary }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left, 0 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right, 2 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op, 3 }, tokens[2], 2);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::StackEnd }, tokens[3], 3);
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Binary",
        "expanded": null,
        "left": {
            "$ast": "NumExpr",
            "value": "1"
        },
        "op": "Add",
        "right": {
            "$ast": "NumExpr",
            "value": "2"
        }
    },
    "imports": []
})");
	});

	TEST_CASE(L"import ... export ...")
	{
		WString input = LR"(
import max
import sum
export sum(1, 2, max(3, 4))
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 18);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::StackBegin }, tokens[0], 0); // <+MODULE>
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[0], 0); // import
			BuildImport(receiver, tokens, 1);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[1], 1);
			receiver.Execute({ AstInsType::Token, -1, 0 }, tokens[2], 2); // import
			BuildImport(receiver, tokens, 3);
			receiver.Execute({ AstInsType::StackSlot, -1, 1 }, tokens[3], 3);
			receiver.Execute({ AstInsType::Token, -1, 2 }, tokens[4], 4); // export

			receiver.Execute({ AstInsType::StackBegin }, tokens[5], 5); // <+CALL>
			BuildRef(receiver, tokens, 5); // sum
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[5], 5);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[6], 6); // '('
			BuildNumExpr(receiver, tokens, 7); // 1
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[7], 7);
			receiver.Execute({ AstInsType::Token, -1, 3 }, tokens[8], 8); // ','
			BuildNumExpr(receiver, tokens, 9); // 2
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[9], 9);
			receiver.Execute({ AstInsType::Token, -1, 3 }, tokens[10], 10); // ','

			receiver.Execute({ AstInsType::StackBegin }, tokens[11], 11); // <+CALL>
			BuildRef(receiver, tokens, 11); // max
			receiver.Execute({ AstInsType::StackSlot, -1, 0 }, tokens[11], 11);
			receiver.Execute({ AstInsType::Token, -1, 1 }, tokens[12], 12); // '('
			BuildNumExpr(receiver, tokens, 13); // 3
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[13], 13);
			receiver.Execute({ AstInsType::Token, -1, 3 }, tokens[14], 14); // ','
			BuildNumExpr(receiver, tokens, 15); // 4
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[15], 15);
			receiver.Execute({ AstInsType::Token, -1, 4 }, tokens[16], 16); // ')'
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Call }, tokens[16], 16);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func, 0 }, tokens[16], 16);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args, 2 }, tokens[16], 16);
			receiver.Execute({ AstInsType::StackEnd }, tokens[16], 16); // <-CALL>
			receiver.Execute({ AstInsType::StackSlot, -1, 2 }, tokens[16], 16);

			receiver.Execute({ AstInsType::Token, -1, 4 }, tokens[17], 17); // ')'
			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Call }, tokens[17], 17);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func, 0 }, tokens[17], 17);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args, 1 }, tokens[17], 17);
			receiver.Execute({ AstInsType::StackEnd }, tokens[17], 17); // <-CALL>
			receiver.Execute({ AstInsType::StackSlot, -1, 3 }, tokens[17], 17);

			receiver.Execute({ AstInsType::CreateObject, (vint32_t)CalculatorClasses::Module }, tokens[17], 17);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_imports, 1 }, tokens[17], 17);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported, 3 }, tokens[17], 17);
			receiver.Execute({ AstInsType::StackEnd }, tokens[17], 17); // <-MODULE>
		}
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Call",
        "args": [{
            "$ast": "NumExpr",
            "value": "1"
        }, {
            "$ast": "NumExpr",
            "value": "2"
        }, {
            "$ast": "Call",
            "args": [{
                "$ast": "NumExpr",
                "value": "3"
            }, {
                "$ast": "NumExpr",
                "value": "4"
            }],
            "func": {
                "$ast": "Ref",
                "name": "max"
            }
        }],
        "func": {
            "$ast": "Ref",
            "name": "sum"
        }
    },
    "imports": [{
        "$ast": "Import",
        "name": "max"
    }, {
        "$ast": "Import",
        "name": "sum"
    }]
})");
	});

#undef LEXER
}
