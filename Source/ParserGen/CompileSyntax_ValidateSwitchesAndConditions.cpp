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
CollectClauseDirectlyTestedSwitchesVisitor
***********************************************************************/

			class CollectClauseDirectlyTestedSwitchesVisitor
				: public Object
				, protected virtual GlrCondition::IVisitor
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&							context;
				Group<RuleSymbol*, RuleSymbol*>&		accessedRules;
				RuleSymbol*								ruleSymbol = nullptr;
				GlrClause*								clause = nullptr;

			public:
				CollectClauseDirectlyTestedSwitchesVisitor(
					VisitorContext& _context,
					Group<RuleSymbol*, RuleSymbol*>& _accessedRules
				)
					: context(_context)
					, accessedRules(_accessedRules)
				{
				}

				void ValidateRule(Ptr<GlrRule> rule)
				{
					ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					for (auto clause : rule->clauses)
					{
						this->clause = clause.Obj();
						this->clause->Accept(this);
					}
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrCondition::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefCondition* node) override
				{
					if (!context.clauseDirectTestedSwitches.Contains(clause, node->name.value))
					{
						context.clauseDirectTestedSwitches.Add(clause, node->name.value);
					}
				}

				void Visit(GlrNotCondition* node) override
				{
					node->condition->Accept(this);
				}

				void Visit(GlrAndCondition* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrOrCondition* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void VisitRuleSymbol(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index != -1)
					{
						auto refRuleSymbol = context.syntaxManager.Rules().Values()[index];
						if (ruleSymbol != refRuleSymbol && !accessedRules.Contains(ruleSymbol, refRuleSymbol))
						{
							accessedRules.Add(ruleSymbol, refRuleSymbol);
						}
					}
				}

				void Visit(GlrRefSyntax* node) override
				{
					VisitRuleSymbol(node->literal.value);
				}

				void Visit(GlrUseSyntax* node) override
				{
					VisitRuleSymbol(node->name.value);
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					if (node->delimiter)
					{
						node->delimiter->Accept(this);
					}
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					for (auto&& branch : node->branches)
					{
						if (branch->condition)
						{
							branch->condition->Accept(this);
						}
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
					}
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
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
				}
			};

/***********************************************************************
CollectRuleAffectedSwitchesVisitor
***********************************************************************/

			class CollectRuleAffectedSwitchesVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&							context;

			public:
				CollectRuleAffectedSwitchesVisitor(VisitorContext& _context)
					: context(_context)
				{
				}

				void ValidateRule(Ptr<GlrRule> rule)
				{
					for (auto clause : rule->clauses)
					{
						clause->Accept(this);
					}
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void VisitRule(RuleSymbol* ruleSymbol)
				{
				}

				void Visit(GlrRefSyntax* node) override
				{
				}

				void Visit(GlrUseSyntax* node) override
				{
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					if (node->delimiter)
					{
						node->delimiter->Accept(this);
					}
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					for (auto&& branch : node->branches)
					{
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
					}
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
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
				}
			};

/***********************************************************************
ValidateStructure
***********************************************************************/

			void ValidateSwitchesAndConditions(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				Group<RuleSymbol*, RuleSymbol*> accessedRules;
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						CollectClauseDirectlyTestedSwitchesVisitor visitor(context, accessedRules);
						visitor.ValidateRule(rule);
					}
				}

				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						CollectRuleAffectedSwitchesVisitor visitor(context);
						visitor.ValidateRule(rule);
					}
				}
			}
		}
	}
}