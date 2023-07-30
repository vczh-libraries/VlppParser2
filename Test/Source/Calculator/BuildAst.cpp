#include "../../../Source/Ast/AstSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

void GenerateCalculatorAst(AstSymbolManager& manager)
{
	{
		auto _group = manager.CreateFileGroup(L"ExprAst");
		auto _ast = _group->CreateFile(L"Ast");
		Fill(_group->cppNss, L"calculator");
		Fill(_group->refNss, L"calculator");
		_group->classPrefix = L"";

		auto _expr = _ast->CreateClass(L"Expr");

		auto _num = _ast->CreateClass(L"NumExpr");
		_num->SetBaseClass(L"Expr");
		_num->CreateProp(L"value")->SetPropType(AstPropType::Token);

		auto _ref = _ast->CreateClass(L"Ref");
		_ref->SetBaseClass(L"Expr");
		_ref->CreateProp(L"name")->SetPropType(AstPropType::Token);

		auto _true = _ast->CreateClass(L"True");
		_true->SetBaseClass(L"Expr");

		auto _false = _ast->CreateClass(L"False");
		_false->SetBaseClass(L"Expr");

		auto _arg = _ast->CreateClass(L"Arg");
		_arg->CreateProp(L"name")->SetPropType(AstPropType::Token);

		auto _Func = _ast->CreateClass(L"Func");
		_Func->SetBaseClass(L"Expr");
		_Func->CreateProp(L"args")->SetPropType(AstPropType::Array, L"Arg");
		_Func->CreateProp(L"value")->SetPropType(AstPropType::Type, L"Expr");

		auto _Call = _ast->CreateClass(L"Call");
		_Call->SetBaseClass(L"Expr");
		_Call->CreateProp(L"func")->SetPropType(AstPropType::Type, L"Expr");
		_Call->CreateProp(L"args")->SetPropType(AstPropType::Array, L"Expr");

		auto _Expandable = _ast->CreateClass(L"Expandable");
		_Expandable->SetBaseClass(L"Expr");
		_Expandable->CreateProp(L"expanded")->SetPropType(AstPropType::Type, L"Expr");

		auto _letExpr = _ast->CreateClass(L"LetExpr");
		_letExpr->SetBaseClass(L"Expandable");
		_letExpr->CreateProp(L"name")->SetPropType(AstPropType::Token);
		_letExpr->CreateProp(L"value")->SetPropType(AstPropType::Type, L"Expr");
		_letExpr->CreateProp(L"result")->SetPropType(AstPropType::Type, L"Expr");

		auto _unaryOp = _ast->CreateEnum(L"UnaryOp");
		_unaryOp->CreateItem(L"Positive");
		_unaryOp->CreateItem(L"Negative");

		auto _unary = _ast->CreateClass(L"Unary");
		_unary->SetBaseClass(L"Expandable");
		_unary->CreateProp(L"op")->SetPropType(AstPropType::Type, L"UnaryOp");
		_unary->CreateProp(L"operand")->SetPropType(AstPropType::Type, L"Expr");

		auto _binaryOp = _ast->CreateEnum(L"BinaryOp");
		_binaryOp->CreateItem(L"Add");
		_binaryOp->CreateItem(L"Minus");
		_binaryOp->CreateItem(L"Multiply");
		_binaryOp->CreateItem(L"Divid");
		_binaryOp->CreateItem(L"GT");
		_binaryOp->CreateItem(L"GE");
		_binaryOp->CreateItem(L"LT");
		_binaryOp->CreateItem(L"LE");
		_binaryOp->CreateItem(L"EQ");
		_binaryOp->CreateItem(L"NE");

		auto _binary = _ast->CreateClass(L"Binary");
		_binary->SetBaseClass(L"Expandable");
		_binary->CreateProp(L"op")->SetPropType(AstPropType::Type, L"BinaryOp");
		_binary->CreateProp(L"left")->SetPropType(AstPropType::Type, L"Expr");
		_binary->CreateProp(L"right")->SetPropType(AstPropType::Type, L"Expr");

		auto _import = _ast->CreateClass(L"Import");
		_import->CreateProp(L"name")->SetPropType(AstPropType::Token);

		auto _module = _ast->CreateClass(L"Module");
		_module->CreateProp(L"imports")->SetPropType(AstPropType::Array, L"Import");
		_module->CreateProp(L"exported")->SetPropType(AstPropType::Type, L"Expr");
	}
}