#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

/***********************************************************************
DirectFirstSetVisitor
***********************************************************************/

			class DirectFirstSetVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			private:
				bool						couldBeEmpty = false;

			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					currentClause = nullptr;

				RuleSymbol* TryGetRuleSymbol(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index == -1) return nullptr;
					return context.syntaxManager.Rules().Values()[index];
				}

			public:
				DirectFirstSetVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void VisitClause(Ptr<GlrClause> clause)
				{
					currentClause = clause.Obj();
					clause->Accept(this);
				}
			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					couldBeEmpty = false;
				}

				void Visit(GlrUseSyntax* node) override
				{
					couldBeEmpty = false;
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					couldBeEmpty = true;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
					couldBeEmpty = true;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					if (couldBeEmpty) node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					bool firstCouldBeEmpty = couldBeEmpty;
					node->second->Accept(this);
					bool secondCouldBeEmpty = couldBeEmpty;
					couldBeEmpty = firstCouldBeEmpty || secondCouldBeEmpty;
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					CHECK_FAIL(L"GlrPushConditionSyntax should have been removed after RewriteSyntax_Switch()!");
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					CHECK_FAIL(L"GlrTestConditionSyntax should have been removed after RewriteSyntax_Switch()!");
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrCreateClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrReuseClause* node) override
				{
					node->syntax->Accept(this);
					if (node->assignments.Count() == 0)
					{
						auto nodeSyntax = node->syntax.Obj();
						auto pushSyntax = dynamic_cast<GlrPushConditionSyntax*>(nodeSyntax);
						if (pushSyntax) nodeSyntax = pushSyntax->syntax.Obj();
					}
				}
			};

/***********************************************************************
CalculateFirstSet
***********************************************************************/

			void CalculateFirstSet_DirectStartRules(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
				{
					auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					DirectFirstSetVisitor visitor(context, ruleSymbol);
					for (auto clause : rule->clauses)
					{
						visitor.VisitClause(clause);
					}
				}
			}

			template<typename TClause>
			void CalculateFirstSet_MoveFromDirectClauses(
				SortedList<TClause*>& clauses,
				Group<RuleSymbol*, TClause*>& indirectClauses,
				Group<RuleSymbol*, TClause*>& directClauses,
				RuleSymbol* rule,
				RuleSymbol* startRule
			)
			{
				vint index = directClauses.Keys().IndexOf(startRule);
				if (index != -1)
				{
					for (auto clause : directClauses.GetByIndex(index))
					{
						if (!clauses.Contains(clause))
						{
							clauses.Add(clause);
							indirectClauses.Add(rule, clause);
						}
					}
				}
			}

			void CalculateFirstSet(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				CalculateFirstSet_DirectStartRules(context, syntaxFile);
			}
		}
	}
}