#include "SyntaxAstToCode.h"

using namespace vl::collections;

class SyntaxAstToStringVisitor
	: public Object
	, protected virtual GlrSyntax::IVisitor
	, protected virtual GlrClause::IVisitor
{
protected:
	TextWriter&					writer;

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
	}

	////////////////////////////////////////////////////////////////////////
	// GlrSyntax::IVisitor
	////////////////////////////////////////////////////////////////////////

	void Visit(GlrRefSyntax* node) override
	{
	}

	void Visit(GlrUseSyntax* node) override
	{
	}

	void Visit(GlrLoopSyntax* node) override
	{
	}

	void Visit(GlrOptionalSyntax* node) override
	{
	}

	void Visit(GlrSequenceSyntax* node) override
	{
	}

	void Visit(GlrAlternativeSyntax* node) override
	{
	}

	void Visit(GlrPushConditionSyntax* node) override
	{
	}

	void Visit(GlrTestConditionSyntax* node) override
	{
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
		node->syntax->Accept(this);
		writer.WriteString(L" as ");
		writer.WriteString(node->type.value);
		Visit(node->assignments);
	}

	void Visit(GlrPartialClause* node) override
	{
		node->syntax->Accept(this);
		writer.WriteString(L" as partial ");
		writer.WriteString(node->type.value);
		Visit(node->assignments);
	}

	void Visit(GlrReuseClause* node) override
	{
		node->syntax->Accept(this);
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
		writer.WriteChar(L')');

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