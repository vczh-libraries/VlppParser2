#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../Source/Calculator/Parser/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/LogParser.h"

using namespace vl::glr::parsergen;
using namespace calculator;

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
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 2);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[1], 1);
			receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
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

	TEST_CASE(L"export 1 <DiscardObject>")
	{
		WString input = LR"(
export 1
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 2);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			receiver.Execute({ AstInsType::DiscardValue }, tokens[1], 1);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[1], 1);
			receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
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
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::DelayFieldAssignment }, tokens[2], 2);
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[2], 2);
				receiver.Execute({ AstInsType::Token }, tokens[2], 2);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[2], 2);
				receiver.Execute({ AstInsType::EndObject }, tokens[2], 2);
				receiver.Execute({ AstInsType::ReopenObject }, tokens[2], 2);
				receiver.Execute({ AstInsType::EndObject }, tokens[2], 2);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[1], 1);
			receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
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
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
					receiver.Execute({ AstInsType::Token }, tokens[1], 1);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
					receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
					receiver.Execute({ AstInsType::Token }, tokens[3], 3);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
					receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
				receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
					receiver.Execute({ AstInsType::Token }, tokens[1], 1);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
					receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
					receiver.Execute({ AstInsType::Token }, tokens[3], 3);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
					receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
				receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
				receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
					receiver.Execute({ AstInsType::Token }, tokens[1], 1);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
					receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
					receiver.Execute({ AstInsType::Token }, tokens[3], 3);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
					receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
				receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
				receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Multiply }, tokens[3], 3);
				receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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

	TEST_CASE(L"export 1 + 2 <DelayFieldAssignment + FieldIfUnassigned>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::DelayFieldAssignment }, tokens[1], 1);
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
				receiver.Execute({ AstInsType::Token }, tokens[3], 3);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::ReopenObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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

	TEST_CASE(L"export 1 + 2 <DelayFieldAssignment + FieldIfUnassigned (canceled)>")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::DelayFieldAssignment }, tokens[1], 1);
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
				receiver.Execute({ AstInsType::Token }, tokens[3], 3);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
			receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Multiply }, tokens[3], 3);
			receiver.Execute({ AstInsType::FieldIfUnassigned, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::ReopenObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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

	TEST_CASE(L"export 1 + 2 (left recursively)")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			{
				receiver.Execute({ AstInsType::BeginObjectLeftRecursive, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
					receiver.Execute({ AstInsType::Token }, tokens[3], 3);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
					receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
				receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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

	TEST_CASE(L"export 1 + 2 (left recursion injection)")
	{
		WString input = LR"(
export 1 + 2
)";
		LEXER(input, tokens);
		TEST_ASSERT(tokens.Count() == 4);
		CalculatorAstInsReceiver receiver;
		{
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::DelayFieldAssignment }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1], 1);
					receiver.Execute({ AstInsType::Token }, tokens[1], 1);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1], 1);
					receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
				}
				receiver.Execute({ AstInsType::LriStore }, tokens[1], 1);
				receiver.Execute({ AstInsType::DelayFieldAssignment }, tokens[1], 1);
				receiver.Execute({ AstInsType::LriFetch }, tokens[1], 1);
				{
					receiver.Execute({ AstInsType::BeginObjectLeftRecursive, (vint32_t)CalculatorClasses::Binary }, tokens[1], 1);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_left }, tokens[1], 1);
					{
						receiver.Execute({ AstInsType::DelayFieldAssignment }, tokens[3], 3);
						receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[3], 3);
						receiver.Execute({ AstInsType::Token }, tokens[3], 3);
						receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[3], 3);
						receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
						receiver.Execute({ AstInsType::ReopenObject }, tokens[3], 3);
						receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
					}
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_right }, tokens[3], 3);
					receiver.Execute({ AstInsType::EnumItem,(vint32_t)BinaryOp::Add }, tokens[3], 3);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Binary_op }, tokens[3], 3);
					receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				}
				receiver.Execute({ AstInsType::ReopenObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::ReopenObject }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[3], 3);
			receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
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
			receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0], 0);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Import }, tokens[1], 1);
				receiver.Execute({ AstInsType::Token }, tokens[1], 1);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Import_name }, tokens[1], 1);
				receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_imports }, tokens[1], 1);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Import }, tokens[3], 3);
				receiver.Execute({ AstInsType::Token }, tokens[3], 3);
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Import_name }, tokens[3], 3);
				receiver.Execute({ AstInsType::EndObject }, tokens[3], 3);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_imports }, tokens[3], 3);
			{
				receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Call }, tokens[5], 5);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Ref }, tokens[5], 5);
					receiver.Execute({ AstInsType::Token }, tokens[5], 5);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Ref_name }, tokens[5], 5);
					receiver.Execute({ AstInsType::EndObject }, tokens[5], 5);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func }, tokens[5], 5);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[7], 7);
					receiver.Execute({ AstInsType::Token }, tokens[7], 7);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[7], 7);
					receiver.Execute({ AstInsType::EndObject }, tokens[7], 7);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args }, tokens[7], 7);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[9], 9);
					receiver.Execute({ AstInsType::Token }, tokens[9], 9);
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[9], 9);
					receiver.Execute({ AstInsType::EndObject }, tokens[9], 9);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args }, tokens[9], 9);
				{
					receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Call }, tokens[11], 11);
					{
						receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Ref }, tokens[11], 11);
						receiver.Execute({ AstInsType::Token }, tokens[11], 11);
						receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Ref_name }, tokens[11], 11);
						receiver.Execute({ AstInsType::EndObject }, tokens[11], 11);
					}
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_func }, tokens[11], 11);
					{
						receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[13], 13);
						receiver.Execute({ AstInsType::Token }, tokens[13], 13);
						receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[13], 13);
						receiver.Execute({ AstInsType::EndObject }, tokens[13], 13);
					}
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args }, tokens[13], 13);
					{
						receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[15], 15);
						receiver.Execute({ AstInsType::Token }, tokens[15], 15);
						receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[15], 15);
						receiver.Execute({ AstInsType::EndObject }, tokens[15], 15);
					}
					receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args }, tokens[15], 15);
					receiver.Execute({ AstInsType::EndObject }, tokens[16], 16);
				}
				receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Call_args }, tokens[16], 16);
				receiver.Execute({ AstInsType::EndObject }, tokens[17], 17);
			}
			receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[17], 17);
			receiver.Execute({ AstInsType::EndObject }, tokens[1], 1);
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