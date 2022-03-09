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

				///////////////////////////////////////////////////////////////////////////////////
				// Condition
				///////////////////////////////////////////////////////////////////////////////////

				auto _Condition = _ast->CreateClass(L"Condition");

				auto _RefCondition = _ast->CreateClass(L"RefCondition");
				_RefCondition->SetBaseClass(L"Condition");
				_RefCondition->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _NotCondition = _ast->CreateClass(L"NotCondition");
				_NotCondition->SetBaseClass(L"Condition");
				_NotCondition->CreateProp(L"condition")->SetPropType(AstPropType::Type, L"Condition");

				auto _AndCondition = _ast->CreateClass(L"AndCondition");
				_AndCondition->SetBaseClass(L"Condition");
				_AndCondition->CreateProp(L"first")->SetPropType(AstPropType::Type, L"Condition");
				_AndCondition->CreateProp(L"second")->SetPropType(AstPropType::Type, L"Condition");

				auto _OrCondition = _ast->CreateClass(L"OrCondition");
				_OrCondition->SetBaseClass(L"Condition");
				_OrCondition->CreateProp(L"first")->SetPropType(AstPropType::Type, L"Condition");
				_OrCondition->CreateProp(L"second")->SetPropType(AstPropType::Type, L"Condition");

				///////////////////////////////////////////////////////////////////////////////////
				// Switch
				///////////////////////////////////////////////////////////////////////////////////

				auto _SwitchValue = _ast->CreateEnum(L"SwitchValue");
				_SwitchValue->CreateItem(L"False");
				_SwitchValue->CreateItem(L"True");

				auto _Switch = _ast->CreateClass(L"SwitchItem");
				_Switch->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_Switch->CreateProp(L"value")->SetPropType(AstPropType::Type, L"SwitchValue");

				///////////////////////////////////////////////////////////////////////////////////
				// Syntax
				///////////////////////////////////////////////////////////////////////////////////

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

				///////////////////////////////////////////////////////////////////////////////////
				// Conditional Clause
				///////////////////////////////////////////////////////////////////////////////////

				auto _PushConditionSyntax = _ast->CreateClass(L"PushConditionSyntax");
				_PushConditionSyntax->SetBaseClass(L"Syntax");
				_PushConditionSyntax->CreateProp(L"switches")->SetPropType(AstPropType::Array, L"SwitchItem");
				_PushConditionSyntax->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");

				auto _TestConditionBranch = _ast->CreateClass(L"TestConditionBranch");
				_TestConditionBranch->CreateProp(L"condition")->SetPropType(AstPropType::Type, L"Condition");
				_TestConditionBranch->CreateProp(L"syntax")->SetPropType(AstPropType::Type, L"Syntax");

				auto _TestConditionSyntax = _ast->CreateClass(L"TestConditionSyntax");
				_TestConditionSyntax->SetBaseClass(L"Syntax");
				_TestConditionSyntax->CreateProp(L"branches")->SetPropType(AstPropType::Array, L"TestConditionBranch");

				///////////////////////////////////////////////////////////////////////////////////
				// Clause
				///////////////////////////////////////////////////////////////////////////////////

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

				///////////////////////////////////////////////////////////////////////////////////
				// Left Recursion Clauses
				///////////////////////////////////////////////////////////////////////////////////

				auto _Lrp = _ast->CreateClass(L"LeftRecursionPlaceholder");
				_Lrp->CreateProp(L"flag")->SetPropType(AstPropType::Token);

				auto _LrpClause = _ast->CreateClass(L"LeftRecursionPlaceholderClause");
				_LrpClause->SetBaseClass(L"Clause");
				_LrpClause->CreateProp(L"flags")->SetPropType(AstPropType::Array, L"LeftRecursionPlaceholder");

				auto _LriClause = _ast->CreateClass(L"LeftRecursionInjectClause");
				_LriClause->SetBaseClass(L"Clause");
				_LriClause->CreateProp(L"flag")->SetPropType(AstPropType::Token);
				_LriClause->CreateProp(L"rule")->SetPropType(AstPropType::Type, L"RefSyntax");
				_LriClause->CreateProp(L"injectionTargets")->SetPropType(AstPropType::Array, L"RefSyntax");

				///////////////////////////////////////////////////////////////////////////////////
				// Rule
				///////////////////////////////////////////////////////////////////////////////////

				auto _Rule = _ast->CreateClass(L"Rule");
				_Rule->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_Rule->CreateProp(L"clauses")->SetPropType(AstPropType::Array, L"Clause");

				///////////////////////////////////////////////////////////////////////////////////
				// File
				///////////////////////////////////////////////////////////////////////////////////

				auto _File = _ast->CreateClass(L"SyntaxFile");
				_File->CreateProp(L"switches")->SetPropType(AstPropType::Array, L"SwitchItem");
				_File->CreateProp(L"rules")->SetPropType(AstPropType::Array, L"Rule");

				return _ast;
			}
		}
	}
}