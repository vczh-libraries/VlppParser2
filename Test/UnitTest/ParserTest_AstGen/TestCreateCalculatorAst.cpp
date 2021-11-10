#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();

TEST_FILE
{
	TEST_CASE(L"CreateParserGenAst")
	{
		AstSymbolManager manager;
		manager.name = L"Calculator";
		manager.cppNss.Add(L"calculator");
		manager.headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATOR";
		{
			auto file = manager.CreateFile(L"Ast");
			{
				file->includes.Add(L"../../../Source/AstBase.h");
				file->cppNss.Add(L"calculator");
				file->refNss.Add(L"calculator");
				file->classPrefix = L"";
				file->headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATORAST";

				auto _expr = file->CreateClass(L"Expr");

				auto _num = file->CreateClass(L"NumExpr");
				_num->SetBaseClass(L"Expr");
				_num->CreateProp(L"value")->SetPropType(AstPropType::Token);

				auto _ref = file->CreateClass(L"Ref");
				_ref->SetBaseClass(L"Expr");
				_ref->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _true = file->CreateClass(L"True");
				_true->SetBaseClass(L"Expr");

				auto _false = file->CreateClass(L"False");
				_false->SetBaseClass(L"Expr");

				auto _arg = file->CreateClass(L"Arg");
				_arg->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _Func = file->CreateClass(L"Func");
				_Func->SetBaseClass(L"Expr");
				_Func->CreateProp(L"args")->SetPropType(AstPropType::Array, L"Arg");
				_Func->CreateProp(L"value")->SetPropType(AstPropType::Type, L"Expr");

				auto _Call = file->CreateClass(L"Call");
				_Call->SetBaseClass(L"Expr");
				_Call->CreateProp(L"func")->SetPropType(AstPropType::Type, L"Expr");
				_Call->CreateProp(L"arg")->SetPropType(AstPropType::Type, L"Expr");

				auto _Expandable = file->CreateClass(L"Expandable");
				_Expandable->SetBaseClass(L"Expr");
				_Expandable->CreateProp(L"expanded")->SetPropType(AstPropType::Type, L"Expr");

				auto _letExpr = file->CreateClass(L"LetExpr");
				_letExpr->SetBaseClass(L"Expandable");
				_letExpr->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_letExpr->CreateProp(L"value")->SetPropType(AstPropType::Type, L"Expr");

				auto _unaryOp = file->CreateEnum(L"UnaryOp");
				_unaryOp->CreateItem(L"Positive");
				_unaryOp->CreateItem(L"Negative");

				auto _unary = file->CreateClass(L"Unary");
				_unary->SetBaseClass(L"Expandable");
				_unary->CreateProp(L"op")->SetPropType(AstPropType::Type, L"UnaryOp");
				_unary->CreateProp(L"operand")->SetPropType(AstPropType::Type, L"Expr");

				auto _binaryOp = file->CreateEnum(L"BinaryOp");
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

				auto _binary = file->CreateClass(L"Binary");
				_binary->SetBaseClass(L"Expandable");
				_binary->CreateProp(L"op")->SetPropType(AstPropType::Type, L"BinaryOp");
				_binary->CreateProp(L"left")->SetPropType(AstPropType::Type, L"Expr");
				_binary->CreateProp(L"right")->SetPropType(AstPropType::Type, L"Expr");

				auto _import = file->CreateClass(L"Import");
				_import->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _module = file->CreateClass(L"Module");
				_module->CreateProp(L"imports")->SetPropType(AstPropType::Array, L"Import");
				_module->CreateProp(L"exported")->SetPropType(AstPropType::Type, L"Expr");
			}
		}
		TEST_ASSERT(manager.Errors().Count() == 0);

		Dictionary<WString, WString> files;
		WriteAstFiles(manager, files);
		auto outputDir = FilePath(GetExePath()) / L"../../Source/CalculatorAst/";
		for (auto [key, index] : indexed(files.Keys()))
		{
			File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
		}
	});
}