/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:RuleAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "ParserGenRuleAst_Json.h"

namespace vl::glr::parsergen::json_visitor
{
	void RuleAstVisitor::PrintFields(GlrAlternativeSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"first"));
		Print(node->first.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"second"));
		Print(node->second.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrAndCondition* node)
	{
		BeginField(vl::WString::Unmanaged(L"first"));
		Print(node->first.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"second"));
		Print(node->second.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrAssignment* node)
	{
		BeginField(vl::WString::Unmanaged(L"field"));
		WriteToken(node->field);
		EndField();
		BeginField(vl::WString::Unmanaged(L"type"));
		switch (node->type)
		{
		case vl::glr::parsergen::GlrAssignmentType::Strong:
			WriteString(vl::WString::Unmanaged(L"Strong"));
			break;
		case vl::glr::parsergen::GlrAssignmentType::Weak:
			WriteString(vl::WString::Unmanaged(L"Weak"));
			break;
		default:
			WriteNull();
		}
		EndField();
		BeginField(vl::WString::Unmanaged(L"value"));
		WriteToken(node->value);
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrClause* node)
	{
	}
	void RuleAstVisitor::PrintFields(GlrCondition* node)
	{
	}
	void RuleAstVisitor::PrintFields(GlrCreateClause* node)
	{
		BeginField(vl::WString::Unmanaged(L"assignments"));
		BeginArray();
		for (auto&& listItem : node->assignments)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"type"));
		WriteToken(node->type);
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrLeftRecursionInjectClause* node)
	{
		BeginField(vl::WString::Unmanaged(L"continuation"));
		Print(node->continuation.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"rule"));
		Print(node->rule.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrLeftRecursionInjectContinuation* node)
	{
		BeginField(vl::WString::Unmanaged(L"configuration"));
		switch (node->configuration)
		{
		case vl::glr::parsergen::GlrLeftRecursionConfiguration::Multiple:
			WriteString(vl::WString::Unmanaged(L"Multiple"));
			break;
		case vl::glr::parsergen::GlrLeftRecursionConfiguration::Single:
			WriteString(vl::WString::Unmanaged(L"Single"));
			break;
		default:
			WriteNull();
		}
		EndField();
		BeginField(vl::WString::Unmanaged(L"flags"));
		BeginArray();
		for (auto&& listItem : node->flags)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"injectionTargets"));
		BeginArray();
		for (auto&& listItem : node->injectionTargets)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"type"));
		switch (node->type)
		{
		case vl::glr::parsergen::GlrLeftRecursionInjectContinuationType::Optional:
			WriteString(vl::WString::Unmanaged(L"Optional"));
			break;
		case vl::glr::parsergen::GlrLeftRecursionInjectContinuationType::Required:
			WriteString(vl::WString::Unmanaged(L"Required"));
			break;
		default:
			WriteNull();
		}
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrLeftRecursionPlaceholder* node)
	{
		BeginField(vl::WString::Unmanaged(L"flag"));
		WriteToken(node->flag);
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrLeftRecursionPlaceholderClause* node)
	{
		BeginField(vl::WString::Unmanaged(L"flags"));
		BeginArray();
		for (auto&& listItem : node->flags)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrLoopSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"delimiter"));
		Print(node->delimiter.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrNotCondition* node)
	{
		BeginField(vl::WString::Unmanaged(L"condition"));
		Print(node->condition.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrOptionalSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"priority"));
		switch (node->priority)
		{
		case vl::glr::parsergen::GlrOptionalPriority::Equal:
			WriteString(vl::WString::Unmanaged(L"Equal"));
			break;
		case vl::glr::parsergen::GlrOptionalPriority::PreferSkip:
			WriteString(vl::WString::Unmanaged(L"PreferSkip"));
			break;
		case vl::glr::parsergen::GlrOptionalPriority::PreferTake:
			WriteString(vl::WString::Unmanaged(L"PreferTake"));
			break;
		default:
			WriteNull();
		}
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrOrCondition* node)
	{
		BeginField(vl::WString::Unmanaged(L"first"));
		Print(node->first.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"second"));
		Print(node->second.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrPartialClause* node)
	{
		BeginField(vl::WString::Unmanaged(L"assignments"));
		BeginArray();
		for (auto&& listItem : node->assignments)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"type"));
		WriteToken(node->type);
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrPrefixMergeClause* node)
	{
		BeginField(vl::WString::Unmanaged(L"rule"));
		Print(node->rule.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrPushConditionSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"switches"));
		BeginArray();
		for (auto&& listItem : node->switches)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrRefCondition* node)
	{
		BeginField(vl::WString::Unmanaged(L"name"));
		WriteToken(node->name);
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrRefSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"field"));
		WriteToken(node->field);
		EndField();
		BeginField(vl::WString::Unmanaged(L"literal"));
		WriteToken(node->literal);
		EndField();
		BeginField(vl::WString::Unmanaged(L"refType"));
		switch (node->refType)
		{
		case vl::glr::parsergen::GlrRefType::ConditionalLiteral:
			WriteString(vl::WString::Unmanaged(L"ConditionalLiteral"));
			break;
		case vl::glr::parsergen::GlrRefType::Id:
			WriteString(vl::WString::Unmanaged(L"Id"));
			break;
		case vl::glr::parsergen::GlrRefType::Literal:
			WriteString(vl::WString::Unmanaged(L"Literal"));
			break;
		default:
			WriteNull();
		}
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrReuseClause* node)
	{
		BeginField(vl::WString::Unmanaged(L"assignments"));
		BeginArray();
		for (auto&& listItem : node->assignments)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrRule* node)
	{
		BeginField(vl::WString::Unmanaged(L"attParser"));
		WriteToken(node->attParser);
		EndField();
		BeginField(vl::WString::Unmanaged(L"attPublic"));
		WriteToken(node->attPublic);
		EndField();
		BeginField(vl::WString::Unmanaged(L"clauses"));
		BeginArray();
		for (auto&& listItem : node->clauses)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"name"));
		WriteToken(node->name);
		EndField();
		BeginField(vl::WString::Unmanaged(L"type"));
		WriteToken(node->type);
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrSequenceSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"first"));
		Print(node->first.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"second"));
		Print(node->second.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrSwitchItem* node)
	{
		BeginField(vl::WString::Unmanaged(L"name"));
		WriteToken(node->name);
		EndField();
		BeginField(vl::WString::Unmanaged(L"value"));
		switch (node->value)
		{
		case vl::glr::parsergen::GlrSwitchValue::False:
			WriteString(vl::WString::Unmanaged(L"False"));
			break;
		case vl::glr::parsergen::GlrSwitchValue::True:
			WriteString(vl::WString::Unmanaged(L"True"));
			break;
		default:
			WriteNull();
		}
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrSyntax* node)
	{
	}
	void RuleAstVisitor::PrintFields(GlrSyntaxFile* node)
	{
		BeginField(vl::WString::Unmanaged(L"rules"));
		BeginArray();
		for (auto&& listItem : node->rules)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
		BeginField(vl::WString::Unmanaged(L"switches"));
		BeginArray();
		for (auto&& listItem : node->switches)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrTestConditionBranch* node)
	{
		BeginField(vl::WString::Unmanaged(L"condition"));
		Print(node->condition.Obj());
		EndField();
		BeginField(vl::WString::Unmanaged(L"syntax"));
		Print(node->syntax.Obj());
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrTestConditionSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"branches"));
		BeginArray();
		for (auto&& listItem : node->branches)
		{
			BeginArrayItem();
			Print(listItem.Obj());
			EndArrayItem();
		}
		EndArray();
		EndField();
	}
	void RuleAstVisitor::PrintFields(GlrUseSyntax* node)
	{
		BeginField(vl::WString::Unmanaged(L"name"));
		WriteToken(node->name);
		EndField();
	}

	void RuleAstVisitor::Visit(GlrRefCondition* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"RefCondition"), node);
		PrintFields(static_cast<GlrCondition*>(node));
		PrintFields(static_cast<GlrRefCondition*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrNotCondition* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"NotCondition"), node);
		PrintFields(static_cast<GlrCondition*>(node));
		PrintFields(static_cast<GlrNotCondition*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrAndCondition* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"AndCondition"), node);
		PrintFields(static_cast<GlrCondition*>(node));
		PrintFields(static_cast<GlrAndCondition*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrOrCondition* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"OrCondition"), node);
		PrintFields(static_cast<GlrCondition*>(node));
		PrintFields(static_cast<GlrOrCondition*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrRefSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"RefSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrRefSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrUseSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"UseSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrUseSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrLoopSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"LoopSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrLoopSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrOptionalSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"OptionalSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrOptionalSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrSequenceSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"SequenceSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrSequenceSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrAlternativeSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"AlternativeSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrAlternativeSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrPushConditionSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"PushConditionSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrPushConditionSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrTestConditionSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"TestConditionSyntax"), node);
		PrintFields(static_cast<GlrSyntax*>(node));
		PrintFields(static_cast<GlrTestConditionSyntax*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrCreateClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"CreateClause"), node);
		PrintFields(static_cast<GlrClause*>(node));
		PrintFields(static_cast<GlrCreateClause*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrPartialClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"PartialClause"), node);
		PrintFields(static_cast<GlrClause*>(node));
		PrintFields(static_cast<GlrPartialClause*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrReuseClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"ReuseClause"), node);
		PrintFields(static_cast<GlrClause*>(node));
		PrintFields(static_cast<GlrReuseClause*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrLeftRecursionPlaceholderClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"LeftRecursionPlaceholderClause"), node);
		PrintFields(static_cast<GlrClause*>(node));
		PrintFields(static_cast<GlrLeftRecursionPlaceholderClause*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrLeftRecursionInjectClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"LeftRecursionInjectClause"), node);
		PrintFields(static_cast<GlrClause*>(node));
		PrintFields(static_cast<GlrLeftRecursionInjectClause*>(node));
		EndObject();
	}

	void RuleAstVisitor::Visit(GlrPrefixMergeClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"PrefixMergeClause"), node);
		PrintFields(static_cast<GlrClause*>(node));
		PrintFields(static_cast<GlrPrefixMergeClause*>(node));
		EndObject();
	}

	RuleAstVisitor::RuleAstVisitor(vl::stream::StreamWriter& _writer)
		: vl::glr::JsonVisitorBase(_writer)
	{
	}

	void RuleAstVisitor::Print(GlrCondition* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		node->Accept(static_cast<GlrCondition::IVisitor*>(this));
	}

	void RuleAstVisitor::Print(GlrSyntax* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		node->Accept(static_cast<GlrSyntax::IVisitor*>(this));
	}

	void RuleAstVisitor::Print(GlrClause* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		node->Accept(static_cast<GlrClause::IVisitor*>(this));
	}

	void RuleAstVisitor::Print(GlrSwitchItem* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"SwitchItem"), node);
		PrintFields(static_cast<GlrSwitchItem*>(node));
		EndObject();
	}

	void RuleAstVisitor::Print(GlrTestConditionBranch* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"TestConditionBranch"), node);
		PrintFields(static_cast<GlrTestConditionBranch*>(node));
		EndObject();
	}

	void RuleAstVisitor::Print(GlrAssignment* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"Assignment"), node);
		PrintFields(static_cast<GlrAssignment*>(node));
		EndObject();
	}

	void RuleAstVisitor::Print(GlrLeftRecursionPlaceholder* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"LeftRecursionPlaceholder"), node);
		PrintFields(static_cast<GlrLeftRecursionPlaceholder*>(node));
		EndObject();
	}

	void RuleAstVisitor::Print(GlrLeftRecursionInjectContinuation* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"LeftRecursionInjectContinuation"), node);
		PrintFields(static_cast<GlrLeftRecursionInjectContinuation*>(node));
		EndObject();
	}

	void RuleAstVisitor::Print(GlrRule* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"Rule"), node);
		PrintFields(static_cast<GlrRule*>(node));
		EndObject();
	}

	void RuleAstVisitor::Print(GlrSyntaxFile* node)
	{
		if (!node)
		{
			WriteNull();
			return;
		}
		BeginObject();
		WriteType(vl::WString::Unmanaged(L"SyntaxFile"), node);
		PrintFields(static_cast<GlrSyntaxFile*>(node));
		EndObject();
	}

}
