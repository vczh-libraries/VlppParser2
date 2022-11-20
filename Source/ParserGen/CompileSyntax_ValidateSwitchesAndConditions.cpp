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
				VisitorSwitchContext&					sContext;
				RuleSymbol*								ruleSymbol = nullptr;
				GlrClause*								currentClause = nullptr;
				List<WString>							pushedSwitches;

			public:
				CollectRuleAffectedSwitchesVisitorBase(
					VisitorContext& _context,
					VisitorSwitchContext& _sContext
				)
					: context(_context)
					, sContext(_sContext)
				{
				}

				void ValidateRule(Ptr<GlrRule> rule)
				{
					ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					for (auto clause : rule->clauses)
					{
						currentClause = clause.Obj();
						clause->Accept(this);
						currentClause = nullptr;
					}
				}

			protected:

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
					VisitorContext& _context,
					VisitorSwitchContext& _sContext
				)
					: CollectRuleAffectedSwitchesVisitorBase(_context, _sContext)
				{
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrCondition::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Traverse(GlrRefCondition* node) override
				{
					if (!pushedSwitches.Contains(node->name.value))
					{
						if (!sContext.clauseAffectedSwitches.Contains(currentClause, node->name.value))
						{
							sContext.clauseAffectedSwitches.Add(currentClause, node->name.value);
						}
						if (!sContext.ruleAffectedSwitches.Contains(ruleSymbol, node->name.value))
						{
							sContext.ruleAffectedSwitches.Add(ruleSymbol, node->name.value);
						}
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
					VisitorContext& _context,
					VisitorSwitchContext& _sContext
				)
					: CollectRuleAffectedSwitchesVisitorBase(_context, _sContext)
				{
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void VisitRule(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index != -1)
					{
						auto refRuleSymbol = context.syntaxManager.Rules().Values()[index];
						vint indexSwitch = sContext.ruleAffectedSwitches.Keys().IndexOf(refRuleSymbol);
						if (indexSwitch != -1)
						{
							for (auto&& name : sContext.ruleAffectedSwitches.GetByIndex(indexSwitch))
							{
								if (!pushedSwitches.Contains(name))
								{
									if (!sContext.clauseAffectedSwitches.Contains(currentClause, name))
									{
										updated = true;
										sContext.clauseAffectedSwitches.Add(currentClause, name);
									}
									if (ruleSymbol != refRuleSymbol && !sContext.ruleAffectedSwitches.Contains(ruleSymbol, name))
									{
										updated = true;
										sContext.ruleAffectedSwitches.Add(ruleSymbol, name);
									}
								}
							}
						}
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
			};

/***********************************************************************
VerifySwitchesAndConditionsVisitor
***********************************************************************/

			class CollectTestedSwitchesVisitor
				: public traverse_visitor::RuleAstVisitor
			{
			protected:
				VisitorContext&							context;
				VisitorSwitchContext&					sContext;

			public:
				SortedList<WString>						testedSwitches;

				CollectTestedSwitchesVisitor(
					VisitorContext& _context,
					VisitorSwitchContext& _sContext
				)
					: context(_context)
					, sContext(_sContext)
				{
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrCondition::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Traverse(GlrRefCondition* node) override
				{
					if (!testedSwitches.Contains(node->name.value))
					{
						testedSwitches.Add(node->name.value);
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void VisitRule(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index != -1)
					{
						auto refRuleSymbol = context.syntaxManager.Rules().Values()[index];
						vint index = sContext.ruleAffectedSwitches.Keys().IndexOf(refRuleSymbol);
						if (index != -1)
						{
							for (auto switchName : sContext.ruleAffectedSwitches.GetByIndex(index))
							{
								if (!testedSwitches.Contains(switchName))
								{
									testedSwitches.Add(switchName);
								}
							}
						}
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
			};

			class VerifySwitchesAndConditionsVisitor
				: public traverse_visitor::RuleAstVisitor
			{
			protected:
				VisitorContext&							context;
				VisitorSwitchContext&					sContext;
				RuleSymbol*								ruleSymbol = nullptr;

			public:
				VerifySwitchesAndConditionsVisitor(
					VisitorContext& _context,
					VisitorSwitchContext& _sContext
				)
					: context(_context)
					, sContext(_sContext)
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

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Traverse(GlrPushConditionSyntax* node) override
				{
					CollectTestedSwitchesVisitor visitor(context, sContext);
					node->syntax->Accept(&visitor);
					for (auto switchItem : node->switches)
					{
						if (!visitor.testedSwitches.Contains(switchItem->name.value))
						{
							context.syntaxManager.AddError(
								ParserErrorType::PushedSwitchIsNotTested,
								node->codeRange,
								ruleSymbol->Name(),
								switchItem->name.value
								);
						}
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Traverse(GlrPrefixMergeClause* node) override
				{
					auto pmRuleSymbol = context.syntaxManager.Rules()[node->rule->literal.value];
					vint index = sContext.ruleAffectedSwitches.Keys().IndexOf(pmRuleSymbol);
					if(index != -1)
					{
						context.syntaxManager.AddError(
							ParserErrorType::PrefixMergeAffectedBySwitches,
							node->codeRange,
							ruleSymbol->Name(),
							pmRuleSymbol->Name(),
							sContext.ruleAffectedSwitches.GetByIndex(index)[0]
							);
					}
				}
			};

/***********************************************************************
ValidateSwitchesAndConditions
***********************************************************************/

			void ValidateSwitchesAndConditions(VisitorContext& context, VisitorSwitchContext& sContext, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
				{
					CollectRuleAffectedSwitchesFirstPassVisitor visitor(context, sContext);
					visitor.ValidateRule(rule);
				}

				while (true)
				{
					CollectRuleAffectedSwitchesSecondPassVisitor visitor(context, sContext);
					for (auto rule : syntaxFile->rules)
					{
						visitor.ValidateRule(rule);
					}

					if (!visitor.updated) break;
				};

				for (auto rule : syntaxFile->rules)
				{
					VerifySwitchesAndConditionsVisitor visitor(context, sContext);
					visitor.ValidateRule(rule);
				}
			}
		}
	}
}