#include "AstToCode.h"

namespace vl::glr::parsergen
{
	using namespace collections;
	using namespace stream;
	
	namespace ast_printer
	{
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
		
			void VisitSwitchItems(List<Ptr<GlrSwitchItem>>& switches)
			{
				for (auto [switchItem, index] : indexed(switches))
				{
					if (index != 0) writer.WriteString(L", ");
					if (switchItem->value == GlrSwitchValue::False) writer.WriteChar(L'!');
					writer.WriteString(switchItem->name.value);
				}
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
				default:;
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
				default:;
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
				VisitSwitchItems(node->switches);
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
		};
	}
	
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
	
		ast_printer::SyntaxAstToStringVisitor visitor(writer);
		for (auto rule : file->rules)
		{
			if (rule->attPublic)
			{
				writer.WriteString(L"@public ");
			}
			if (rule->attParser)
			{
				writer.WriteString(L"@parser ");
			}
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
}