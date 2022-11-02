#include "Compiler.h"
#include "../ParserGen_Generated/ParserGenRuleAst_Traverse.h"

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
				: public traverse_visitor::RuleAstVisitor
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

				void Traverse(GlrRefSyntax* node) override
				{
					VisitRule(node->literal.value);
				}

				void Traverse(GlrUseSyntax* node) override
				{
					VisitRule(node->name.value);
				}

				void Traverse(GlrPushConditionSyntax* node) override
				{
					for (auto switchItem : node->switches)
					{
						pushedSwitches.Add(switchItem->name.value);
					}
				}

				void Finishing(GlrPushConditionSyntax* node) override
				{
					for (vint i = 0; i < node->switches.Count(); i++)
					{
						pushedSwitches.RemoveAt(pushedSwitches.Count() - 1);
					}
				}
			};

/***********************************************************************
CollectRuleAffectedSwitchesFirstPassVisitor
***********************************************************************/

			class CollectRuleAffectedSwitchesFirstPassVisitor
				: public CollectRuleAffectedSwitchesVisitorBase
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

				void Traverse(GlrRefCondition* node) override
				{
					if (!context.ruleAffectedSwitches.Contains(ruleSymbol, node->name.value))
					{
						context.ruleAffectedSwitches.Add(ruleSymbol, node->name.value);
					}
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