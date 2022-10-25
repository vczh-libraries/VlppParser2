#include "SyntaxAstToCode.h"

using namespace vl::collections;

class SyntaxAstToStringVisitor
	: public Object
	, protected virtual GlrCondition::IVisitor
	, protected virtual GlrSyntax::IVisitor
	, protected virtual GlrClause::IVisitor
{
protected:
	TextWriter&					writer;
	vint						priority = -1;

public:
	SyntaxAstToStringVisitor(
		TextWriter& _writer
	)
		: writer(_writer)
	{
	}

	void VisitClause(Ptr<GlrClause> clause)
	{
		clause->Accept(this);
	}
protected:

	void VisitString(const WString& str)
	{
		writer.WriteString(str);
	}

	void VisitConditionalLiteral(const WString& str)
	{
		writer.WriteString(str);
	}

	void VisitSyntax(GlrSyntax* node, vint _priority = 2)
	{
		vint oldPriority = priority;
		priority = _priority;
		node->Accept(this);
		priority = oldPriority;
	}

	void VisitCondition(GlrCondition* node, vint _priority = 2)
	{
		vint oldPriority = priority;
		priority = _priority;
		node->Accept(this);
		priority = oldPriority;
	}

	////////////////////////////////////////////////////////////////////////
	// GlrCondition::IVisitor
	////////////////////////////////////////////////////////////////////////

	void Visit(GlrRefCondition* node) override
	{
		writer.WriteString(node->name.value);
	}

	void Visit(GlrNotCondition* node) override
	{
		writer.WriteChar(L'!');
		VisitCondition(node->condition.Obj(), 0);
	}

	void Visit(GlrAndCondition* node) override
	{
		if (priority < 1) writer.WriteChar(L'(');
		VisitCondition(node->first.Obj(), 1);
		writer.WriteString(L"&&");
		VisitCondition(node->second.Obj(), 1);
		if (priority < 1) writer.WriteChar(L')');
	}

	void Visit(GlrOrCondition* node) override
	{
		if (priority < 2) writer.WriteChar(L'(');
		VisitCondition(node->first.Obj(), 2);
		writer.WriteString(L"||");
		VisitCondition(node->second.Obj(), 2);
		if (priority < 2) writer.WriteChar(L')');
	}

	////////////////////////////////////////////////////////////////////////
	// GlrSyntax::IVisitor
	////////////////////////////////////////////////////////////////////////

	void Visit(GlrRefSyntax* node) override
	{
		switch (node->refType)
		{
		case GlrRefType::Id:
			writer.WriteString(node->literal.value);
			break;
		case GlrRefType::Literal:
			VisitString(node->literal.value);
			break;
		case GlrRefType::ConditionalLiteral:
			VisitConditionalLiteral(node->literal.value);
			break;
		}
		if (node->field)
		{
			writer.WriteChar(L':');
			writer.WriteString(node->field.value);
		}
	}

	void Visit(GlrUseSyntax* node) override
	{
		writer.WriteChar(L'!');
		writer.WriteString(node->name.value);
	}

	void Visit(GlrLoopSyntax* node) override
	{
		writer.WriteChar(L'{');
		VisitSyntax(node->syntax.Obj());
		if (node->delimiter)
		{
			writer.WriteString(L" ; ");
			VisitSyntax(node->delimiter.Obj());
		}
		writer.WriteChar(L'}');
	}

	void Visit(GlrOptionalSyntax* node) override
	{
		switch (node->priority)
		{
		case GlrOptionalPriority::PreferTake:
			writer.WriteChar(L'+');
			break;
		case GlrOptionalPriority::PreferSkip:
			writer.WriteChar(L'-');
			break;
		case GlrOptionalPriority::Equal:
			break;
		}
		writer.WriteChar(L'[');
		VisitSyntax(node->syntax.Obj());
		writer.WriteChar(L']');
	}

	void Visit(GlrSequenceSyntax* node) override
	{
		if (priority < 1) writer.WriteChar(L'(');
		VisitSyntax(node->first.Obj(), 1);
		writer.WriteChar(L' ');
		VisitSyntax(node->second.Obj(), 1);
		if (priority < 1) writer.WriteChar(L')');
	}

	void Visit(GlrAlternativeSyntax* node) override
	{
		if (priority < 2) writer.WriteChar(L'(');
		VisitSyntax(node->first.Obj(), 2);
		writer.WriteString(L" | ");
		VisitSyntax(node->second.Obj(), 2);
		if (priority < 2) writer.WriteChar(L')');
	}

	void Visit(GlrPushConditionSyntax* node) override
	{
		writer.WriteString(L"!(");
		for (auto [switchItem, index] : indexed(node->switches))
		{
			if (index != 0) writer.WriteString(L", ");
			if (switchItem->value == GlrSwitchValue::False) writer.WriteChar(L'!');
			writer.WriteString(switchItem->name.value);
		}
		writer.WriteString(L"; ");
		VisitSyntax(node->syntax.Obj());
		writer.WriteChar(L')');
	}

	void Visit(GlrTestConditionSyntax* node) override
	{
		writer.WriteString(L"?(");
		for (auto [branch, index] : indexed(node->branches))
		{
			if (index != 0) writer.WriteString(L" | ");
			VisitCondition(branch->condition.Obj());
			writer.WriteString(L": ");
			if (branch->syntax)
			{
				VisitSyntax(branch->syntax.Obj());
			}
			else
			{
				writer.WriteChar(L';');
			}
		}
		writer.WriteChar(L')');
	}

	////////////////////////////////////////////////////////////////////////
	// GlrClause::IVisitor
	////////////////////////////////////////////////////////////////////////

	void Visit(List<Ptr<GlrAssignment>>& assignments)
	{
		if (assignments.Count() > 0)
		{
			writer.WriteString(L" {");
			for (auto [assignment, index] : indexed(assignments))
			{
				if (index != 0) writer.WriteString(L", ");
				writer.WriteString(assignment->field.value);
				if (assignment->type == GlrAssignmentType::Weak)
				{
					writer.WriteString(L" ?= ");
				}
				else
				{
					writer.WriteString(L" = ");
				}
				VisitString(assignment->value.value);
			}
			writer.WriteChar(L'}');
		}
	}

	void Visit(GlrCreateClause* node) override
	{
		VisitSyntax(node->syntax.Obj());
		writer.WriteString(L" as ");
		writer.WriteString(node->type.value);
		Visit(node->assignments);
	}

	void Visit(GlrPartialClause* node) override
	{
		VisitSyntax(node->syntax.Obj());
		writer.WriteString(L" as partial ");
		writer.WriteString(node->type.value);
		Visit(node->assignments);
	}

	void Visit(GlrReuseClause* node) override
	{
		VisitSyntax(node->syntax.Obj());
		Visit(node->assignments);
	}

	void Visit(GlrLeftRecursionPlaceholderClause* node) override
	{
		writer.WriteString(L"left_recursion_placeholder(");
		for (auto [flag, index] : indexed(node->flags))
		{
			if (index != 0) writer.WriteString(L", ");
			writer.WriteString(flag->flag.value);
		}
		writer.WriteChar(L')');
	}

	void VisitLriCont(GlrLeftRecursionInjectContinuation* node)
	{
		if (node->type == GlrLeftRecursionInjectContinuationType::Optional)
		{
			writer.WriteChar(L'[');
		}

		writer.WriteString(L"left_recursion_inject");
		if (node->configuration == GlrLeftRecursionConfiguration::Multiple)
		{
			writer.WriteString(L"_multiple");
		}

		writer.WriteChar(L'(');
		writer.WriteString(node->flag->flag.value);
		writer.WriteString(L") ");

		for (auto [target, index] : indexed(node->injectionTargets))
		{
			if (index != 0) writer.WriteString(L" | ");
			VisitLriTarget(target.Obj());
		}

		if (node->type == GlrLeftRecursionInjectContinuationType::Optional)
		{
			writer.WriteChar(L']');
		}
	}

	void VisitLriTarget(GlrLeftRecursionInjectClause* node)
	{
		if (node->continuation)
		{
			writer.WriteChar(L'(');
			writer.WriteString(node->rule->literal.value);
			writer.WriteChar(L' ');
			VisitLriCont(node->continuation.Obj());
			writer.WriteChar(L')');
		}
		else
		{
			writer.WriteString(node->rule->literal.value);
		}
	}

	void Visit(GlrLeftRecursionInjectClause* node) override
	{
		writer.WriteChar(L'!');
		writer.WriteString(node->rule->literal.value);
		writer.WriteChar(L' ');
		VisitLriCont(node->continuation.Obj());
	}

	void Visit(GlrPrefixMergeClause* node) override
	{
		writer.WriteString(L"!prefix_merge(");
		writer.WriteString(node->rule->literal.value);
		writer.WriteString(L")");
	}
};

/***********************************************************************
SyntaxAstToCode
***********************************************************************/

void SyntaxAstToCode(
	Ptr<GlrSyntaxFile> file,
	TextWriter& writer
)
{
	if (file->switches.Count() > 0)
	{
		writer.WriteString(L"switch ");
		for (auto [switchItem, index] : indexed(file->switches))
		{
			if (index != 0) writer.WriteString(L", ");
			if (switchItem->value == GlrSwitchValue::False) writer.WriteChar(L'!');
			writer.WriteString(switchItem->name.value);
		}
		writer.WriteLine(L";");
		writer.WriteLine(L"");
	}

	SyntaxAstToStringVisitor visitor(writer);
	for (auto rule : file->rules)
	{
		writer.WriteString(rule->name.value);
		if (rule->type)
		{
			writer.WriteString(L" : ");
			writer.WriteString(rule->type.value);
		}
		writer.WriteLine(L"");

		for (auto clause : rule->clauses)
		{
			writer.WriteString(L"  ::= ");
			visitor.VisitClause(clause);
			writer.WriteLine(L"");
		}
		writer.WriteLine(L"  ;");
		writer.WriteLine(L"");
	}
}