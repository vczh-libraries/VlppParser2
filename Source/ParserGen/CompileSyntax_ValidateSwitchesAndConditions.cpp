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
CollectRuleAffectedSwitchesVisitorBase
***********************************************************************/

			class CollectRuleAffectedSwitchesVisitorBase
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&							context;
				RuleSymbol*								ruleSymbol = nullptr;
				List<WString>							pushedSwitches;

			public:
				CollectRuleAffectedSwitchesVisitorBase(
					VisitorContext& _context
				)
					: context(_context)
				{
				}

				void ValidateRule(Ptr<GlrRule> rule)
				{
					ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					for (auto clause : rule->clauses)
					{
						clause->Accept(this);
					}
				}

			protected:

				virtual void VisitRuleSymbol(RuleSymbol* refRuleSymbol) = 0;

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void VisitRule(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index != -1)
					{
						VisitRuleSymbol(context.syntaxManager.Rules().Values()[index]);
					}
				}

				void Visit(GlrRefSyntax* node) override
				{
					VisitRule(node->literal.value);
				}

				void Visit(GlrUseSyntax* node) override
				{
					VisitRule(node->name.value);
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
					for (auto switchItem : node->switches)
					{
						pushedSwitches.Add(switchItem->name.value);
					}
					node->syntax->Accept(this);
					for (vint i = 0; i < node->switches.Count(); i++)
					{
						pushedSwitches.RemoveAt(pushedSwitches.Count() - 1);
					}
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
					node->rule->Accept(this);
					if (node->continuation)
					{
						for (auto target : node->continuation->injectionTargets)
						{
							target->Accept(this);
						}
					}
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
					node->rule->Accept(this);
				}
			};

/***********************************************************************
CollectRuleAffectedSwitchesFirstPassVisitor
***********************************************************************/

			class CollectRuleAffectedSwitchesFirstPassVisitor
				: public CollectRuleAffectedSwitchesVisitorBase
				, protected virtual GlrCondition::IVisitor
			{
			public:
				CollectRuleAffectedSwitchesFirstPassVisitor(
					VisitorContext& _context
				)
					: CollectRuleAffectedSwitchesVisitorBase(_context)
				{
				}

			protected:

				void VisitRuleSymbol(RuleSymbol* refRuleSymbol) override
				{
				}

				////////////////////////////////////////////////////////////////////////
				// GlrCondition::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefCondition* node) override
				{
					if (!context.ruleAffectedSwitches.Contains(ruleSymbol, node->name.value))
					{
						context.ruleAffectedSwitches.Add(ruleSymbol, node->name.value);
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

				void Visit(GlrTestConditionSyntax* node) override
				{
					for (auto&& branch : node->branches)
					{
						if (branch->condition)
						{
							branch->condition->Accept(this);
						}
					}
					CollectRuleAffectedSwitchesVisitorBase::Visit(node);
				}
			};

/***********************************************************************
CollectRuleAffectedSwitchesSecondPassVisitor
***********************************************************************/

			class CollectRuleAffectedSwitchesSecondPassVisitor
				: public CollectRuleAffectedSwitchesVisitorBase
			{
			public:
				bool									updated = false;

				CollectRuleAffectedSwitchesSecondPassVisitor(
					VisitorContext& _context
				)
					: CollectRuleAffectedSwitchesVisitorBase(_context)
				{
				}

			protected:

				void VisitRuleSymbol(RuleSymbol* refRuleSymbol) override
				{
					if (ruleSymbol == refRuleSymbol) return;
					vint indexSwitch = context.ruleAffectedSwitches.Keys().IndexOf(refRuleSymbol);
					if(indexSwitch!=-1)
					{
						for (auto&& name : context.ruleAffectedSwitches.GetByIndex(indexSwitch))
						{
							if (!context.ruleAffectedSwitches.Contains(ruleSymbol, name))
							{
								updated = true;
								context.ruleAffectedSwitches.Add(ruleSymbol, name);
							}
						}
					}
				}
			};

/***********************************************************************
ValidateStructure
***********************************************************************/

			void ValidateSwitchesAndConditions(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						CollectRuleAffectedSwitchesFirstPassVisitor visitor(context);
						visitor.ValidateRule(rule);
					}
				}

				while (true)
				{
					CollectRuleAffectedSwitchesSecondPassVisitor visitor(context);
					for (auto file : files)
					{
						for (auto rule : file->rules)
						{
							visitor.ValidateRule(rule);
						}
					}

					if (!visitor.updated) break;
				};
			}
		}
	}
}