#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CreateParserGenRuleAst
***********************************************************************/

			AstDefFile* CreateParserGenRuleAst(AstSymbolManager& manager)
			{
				auto _ast = manager.CreateFile(L"RuleAst");
				Fill(_ast->cppNss, L"vl", L"glr", L"parsergen");
				Fill(_ast->refNss, L"glr", L"parsergen");
				_ast->classPrefix = L"Glr";

				auto _Clause = _ast->CreateClass(L"Clause");

				auto _RefClause = _ast->CreateClass(L"RefClause");
				_RefClause->SetBaseClass(L"Clause");
				_RefClause->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_RefClause->CreateProp(L"field")->SetPropType(AstPropType::Token);

				auto _LiteralClause = _ast->CreateClass(L"LiteralClause");
				_LiteralClause->SetBaseClass(L"Clause");
				_LiteralClause->CreateProp(L"value")->SetPropType(AstPropType::Token);

				auto _UseClause = _ast->CreateClass(L"UseClause");
				_UseClause->SetBaseClass(L"Clause");
				_UseClause->CreateProp(L"clause")->SetPropType(AstPropType::Type, L"Clause");

				auto _LoopClause = _ast->CreateClass(L"LoopClause");
				_LoopClause->SetBaseClass(L"Clause");
				_LoopClause->CreateProp(L"clause")->SetPropType(AstPropType::Type, L"Clause");
				_LoopClause->CreateProp(L"delimiter")->SetPropType(AstPropType::Type, L"Clause");

				auto _OptionalClause = _ast->CreateClass(L"OptionalClause");
				_OptionalClause->SetBaseClass(L"Clause");
				_OptionalClause->CreateProp(L"clause")->SetPropType(AstPropType::Type, L"Clause");

				auto _SequenceClause = _ast->CreateClass(L"SequenceClause");
				_SequenceClause->SetBaseClass(L"Clause");
				_SequenceClause->CreateProp(L"first")->SetPropType(AstPropType::Type, L"Clause");
				_SequenceClause->CreateProp(L"second")->SetPropType(AstPropType::Type, L"Clause");

				auto _Assignment = _ast->CreateClass(L"Assignment");
				_Assignment->CreateProp(L"field")->SetPropType(AstPropType::Token);
				_Assignment->CreateProp(L"value")->SetPropType(AstPropType::Token);

				auto _CreateClause = _ast->CreateClass(L"CreateClause");
				_CreateClause->SetBaseClass(L"Clause");
				_CreateClause->CreateProp(L"type")->SetPropType(AstPropType::Token);
				_CreateClause->CreateProp(L"clause")->SetPropType(AstPropType::Type, L"Clause");
				_CreateClause->CreateProp(L"assignments")->SetPropType(AstPropType::Array, L"Assignment");

				auto _ReuseClause = _ast->CreateClass(L"_ReuseClause");
				_ReuseClause->SetBaseClass(L"Clause");
				_ReuseClause->CreateProp(L"clause")->SetPropType(AstPropType::Type, L"Clause");
				_ReuseClause->CreateProp(L"assignments")->SetPropType(AstPropType::Array, L"Assignment");

				auto _Rule = _ast->CreateClass(L"Rule");
				_Rule->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_Rule->CreateProp(L"clauses")->SetPropType(AstPropType::Array, L"Clause");

				auto _File = _ast->CreateClass(L"SyntaxFile");
				_File->CreateProp(L"rules")->SetPropType(AstPropType::Array, L"Rule");

				return _ast;
			}
		}
	}
}