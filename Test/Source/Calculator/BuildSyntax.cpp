#include "../../../Source/Syntax/SyntaxSymbolWriter.h"
#include "../../../Source/Ast/AstSymbol.h"
#include "Parser/Calculator_Assembler.h"
#include "Parser/Calculator_Lexer.h"

using namespace vl;
using namespace vl::glr::parsergen;
using namespace vl::glr::parsergen::syntax_writer;
using namespace calculator;

auto tok(CalculatorTokens id)
{
	auto d = CalculatorTokenDisplayText(id);
	auto n = CalculatorTokenId(id);
	return syntax_writer::tok(
		id,
		(d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n))
		);
}

auto tok(CalculatorTokens id, CalculatorFields field)
{
	auto d = CalculatorTokenDisplayText(id);
	auto n = CalculatorTokenId(id);
	return syntax_writer::tok(
		id,
		(d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n)),
		field
		);
}

void GenerateCalculatorSyntax(AstSymbolManager& ast, SyntaxSymbolManager& manager)
{
	manager.name = L"ModuleParser";

	auto _arg = manager.CreateRule(L"Arg");
	auto _exp0 = manager.CreateRule(L"Exp0");
	auto _exp1 = manager.CreateRule(L"Exp1");
	auto _exp2 = manager.CreateRule(L"Exp2");
	auto _exp3 = manager.CreateRule(L"Exp3");
	auto _exp4 = manager.CreateRule(L"Exp4");
	auto _exp5 = manager.CreateRule(L"Exp5");
	auto _exp = manager.CreateRule(L"Exp");
	auto _import = manager.CreateRule(L"Import");
	auto _module = manager.CreateRule(L"Module");

	_exp->isParser = true;
	_exp->ruleType = dynamic_cast<AstClassSymbol*>(ast.Symbols()[L"Expr"]);
	_module->isParser = true;
	_module->ruleType = dynamic_cast<AstClassSymbol*>(ast.Symbols()[L"Module"]);

	using T = CalculatorTokens;
	using C = CalculatorClasses;
	using F = CalculatorFields;

	// ID:name as Arg
	Clause{ _arg } = create(tok(T::ID, F::Arg_name), C::Arg);

	// NUM:value as NumExpr
	Clause{ _exp0 } = create(tok(T::NUM, F::NumExpr_value), C::NumExpr);
	// ID:name as Ref
	Clause{ _exp0 } = create(tok(T::ID, F::Ref_name), C::Ref);
	// "true" as True
	Clause{ _exp0 } = create(tok(T::TRUE), C::True);
	// "false" as False
	Clause{ _exp0 } = create(tok(T::FALSE), C::False);
	// "(" !Exp ")"
	Clause{ _exp0 } = tok(T::OPEN_BRACE) + use(_exp) + tok(T::CLOSE_BRACE);

	// !Exp0
	Clause{ _exp1 } = use(_exp0);
	// "+" Exp1:operand as Unary {op = Positive}
	Clause{ _exp1 } = create(tok(T::ADD) + rule(_exp1, F::Unary_operand), C::Unary).with(F::Unary_op, UnaryOp::Positive);
	// "-" Exp1:operand as Unary {op = Negative}
	Clause{ _exp1 } = create(tok(T::SUB) + rule(_exp1, F::Unary_operand), C::Unary).with(F::Unary_op, UnaryOp::Negative);
	// Exp1:func "(" {Exp:args : ","} ")" as Call
	Clause{ _exp1 } = create(rule(_exp1, F::Call_func) + tok(T::OPEN_BRACE) + loop(rule(_exp, F::Call_args), tok(T::COMMA)) + tok(T::CLOSE_BRACE), C::Call);

	// !Exp1
	Clause{ _exp2 } = use(_exp1);
	// Exp2:left "*" Exp1:right as Binary {op = "Multiply"}
	Clause{ _exp2 } = create(rule(_exp2, F::Binary_left) + tok(T::MUL) + rule(_exp1, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::Multiply);
	// Exp2:left "/" Exp1:right as Binary {op = "Divid"}
	Clause{ _exp2 } = create(rule(_exp2, F::Binary_left) + tok(T::DIV) + rule(_exp1, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::Divid);

	// !Exp2
	Clause{ _exp3 } = use(_exp2);
	// Exp3:left "+" Exp2:right as Binary {op = "Add"}
	Clause{ _exp3 } = create(rule(_exp3, F::Binary_left) + tok(T::ADD) + rule(_exp2, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::Add);
	// Exp3:left "-" Exp2:right as Binary {op = "Minus"}
	Clause{ _exp3 } = create(rule(_exp3, F::Binary_left) + tok(T::SUB) + rule(_exp2, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::Minus);

	// !Exp3
	Clause{ _exp4 } = use(_exp3);
	// Exp4:left "<" Exp3:right as Binary {op = "LT"}
	Clause{ _exp4 } = create(rule(_exp4, F::Binary_left) + tok(T::LT) + rule(_exp3, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::LT);
	// Exp4:left "<=" Exp3:right as Binary {op = "LE"}
	Clause{ _exp4 } = create(rule(_exp4, F::Binary_left) + tok(T::LE) + rule(_exp3, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::LE);
	// Exp4:left ">" Exp3:right as Binary {op = "GT"}
	Clause{ _exp4 } = create(rule(_exp4, F::Binary_left) + tok(T::GT) + rule(_exp3, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::GT);
	// Exp4:left ">=" Exp3:right as Binary {op = "GE"}
	Clause{ _exp4 } = create(rule(_exp4, F::Binary_left) + tok(T::GE) + rule(_exp3, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::GE);

	// !Exp4
	Clause{ _exp5 } = use(_exp4);
	// Exp5:left "==" Exp4:right as Binary {op = "EQ"}
	Clause{ _exp5 } = create(rule(_exp5, F::Binary_left) + tok(T::EQ) + rule(_exp4, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::EQ);
	// xp5:left "!=" Exp4:right as Binary {op = "NE"}
	Clause{ _exp5 } = create(rule(_exp5, F::Binary_left) + tok(T::NE) + rule(_exp4, F::Binary_right), C::Binary).with(F::Binary_op, BinaryOp::NE);

	// !Exp5
	Clause{ _exp } = use(_exp5);
	// "(" {Arg:args : ","} ")" "->" Exp:value as Func
	Clause{ _exp } = create(tok(T::OPEN_BRACE) + loop(rule(_arg, F::Func_args), tok(T::COMMA)) + tok(T::CLOSE_BRACE) + tok(T::INFER) + rule(_exp, F::Func_value), C::Func);
	// "let" ID:name "<-" Exp:value "in" Exp:result as LetExpr
	Clause{ _exp } = create(tok(T::LET) + tok(T::ID, F::LetExpr_name) + tok(T::ASSIGN) + rule(_exp, F::LetExpr_value) + tok(T::IN) + rule(_exp, F::LetExpr_result), C::LetExpr);

	// "import" ID:name as Import
	Clause{ _import } = create(tok(T::IMPORT) +  tok(T::ID, F::Import_name), C::Import);

	// {Import:imports} "export" Exp:exported as Module
	Clause{ _module } = create(loop(rule(_import, F::Module_imports)) + tok(T::EXPORT) + rule(_exp, F::Module_exported), C::Module);
}