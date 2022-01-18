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

				auto _Syntax = _ast->CreateClass(L"Syntax");

				auto _RefType = _ast->CreateEnum(L"RefType");
				_RefType->CreateItem(L"Id");
				_RefType->CreateItem(L"Literal");
				_RefType->CreateItem(L"ConditionalLiteral");

				auto _RefSyntax = _ast->CreateClass(L"RefSyntax");
				_RefSyntax->SetBaseClass(L"Syntax");
				_RefSyntax->CreateProp(L"refType")->SetPropType(AstPropType::Type, L"RefType");
				_RefSyntax->CreateProp(L"literal")->SetPropType(AstPropType::Token);
				_RefSyntax->CreateProp(L"field")->SetPropType(AstPropType::Token);

				auto _UseSyntax = _ast->CreateClass(L"UseSyntax");
				_UseSyntax->SetBaseClass(L"Syntax");
				_UseSyntax->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _LoopSyntax = _ast->CreateClass(L"LoopSyntax");
				_LoopSyntax->SetBaseClass(L"Syntax");
				_LoopSyntax->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");
				_LoopSyntax->CreateProp(L"delimiter")->SetPropType(AstPropType::Type, L"Syntax");

				auto _OptionalPriority = _ast->CreateEnum(L"OptionalPriority");
				_OptionalPriority->CreateItem(L"Equal");
				_OptionalPriority->CreateItem(L"PreferTake");
				_OptionalPriority->CreateItem(L"PreferSkip");

				auto _OptionalSyntax = _ast->CreateClass(L"OptionalSyntax");
				_OptionalSyntax->SetBaseClass(L"Syntax");
				_OptionalSyntax->CreateProp(L"priority")->SetPropType(AstPropType::Type, L"OptionalPriority");
				_OptionalSyntax->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");

				auto _SequenceSyntax = _ast->CreateClass(L"SequenceSyntax");
				_SequenceSyntax->SetBaseClass(L"Syntax");
				_SequenceSyntax->CreateProp(L"first")->SetPropType(AstPropType::Type, L"Syntax");
				_SequenceSyntax->CreateProp(L"second")->SetPropType(AstPropType::Type, L"Syntax");

				auto _AlternativeSyntax = _ast->CreateClass(L"AlternativeSyntax");
				_AlternativeSyntax->SetBaseClass(L"Syntax");
				_AlternativeSyntax->CreateProp(L"first")->SetPropType(AstPropType::Type, L"Syntax");
				_AlternativeSyntax->CreateProp(L"second")->SetPropType(AstPropType::Type, L"Syntax");

				auto _Clause = _ast->CreateClass(L"Clause");

				auto _AssignmentType = _ast->CreateEnum(L"AssignmentType");
				_AssignmentType->CreateItem(L"Strong");
				_AssignmentType->CreateItem(L"Weak");

				auto _Assignment = _ast->CreateClass(L"Assignment");
				_Assignment->CreateProp(L"type")->SetPropType(AstPropType::Type, L"AssignmentType");
				_Assignment->CreateProp(L"field")->SetPropType(AstPropType::Token);
				_Assignment->CreateProp(L"value")->SetPropType(AstPropType::Token);

				auto _CreateClause = _ast->CreateClass(L"CreateClause");
				_CreateClause->SetBaseClass(L"Clause");
				_CreateClause->CreateProp(L"type")->SetPropType(AstPropType::Token);
				_CreateClause->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");
				_CreateClause->CreateProp(L"assignments")->SetPropType(AstPropType::Array, L"Assignment");

				auto _PartialClause = _ast->CreateClass(L"PartialClause");
				_PartialClause->SetBaseClass(L"Clause");
				_PartialClause->CreateProp(L"type")->SetPropType(AstPropType::Token);
				_PartialClause->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");
				_PartialClause->CreateProp(L"assignments")->SetPropType(AstPropType::Array, L"Assignment");

				auto _ReuseClause = _ast->CreateClass(L"ReuseClause");
				_ReuseClause->SetBaseClass(L"Clause");
				_ReuseClause->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");
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