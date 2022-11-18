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

			namespace rewritesyntax_switch
			{
				struct GeneratedRule
				{
					Ptr<GlrRule>				ruleToExpand = nullptr;
					Dictionary<WString, bool>	switchValues;
				};

				struct RewritingContext
				{
					Group<RuleSymbol*, Ptr<GeneratedRule>>		generatedRules;
				};

/***********************************************************************
ExpandSwitchSyntaxVisitor
***********************************************************************/

				class ExpandSwitchSyntaxVisitor : public traverse_visitor::RuleAstVisitor
				{
				protected:
					struct Identification
					{
						RuleSymbol*						workingRule = nullptr;
						Ptr<Dictionary<WString, bool>>	workingSwitchValues = nullptr;
						Ptr<GeneratedRule>				generatedRule;
					};

					VisitorContext&						vContext;
					RewritingContext&					rContext;
					SortedList<RuleSymbol*>				scannedUnaffectedRules;
					
					Identification						identification;

				protected:
					void TraverseRule(const ParsingToken& ruleName)
					{
						vint index = vContext.syntaxManager.Rules().Keys().IndexOf(ruleName.value);
						if (index == -1) return;

						auto ruleSymbol = vContext.syntaxManager.Rules().Values()[index];
						auto rule = vContext.astRules[ruleSymbol];

						index = vContext.ruleAffectedSwitches.Keys().IndexOf(ruleSymbol);
						if (index == -1)
						{
							InspectIntoUnaffectedRule(rule);
						}
						else
						{
							InspectIntoAffectedRule(rule, identification.workingSwitchValues);
						}
					}

					void Traverse(GlrRefSyntax* node) override
					{
						if (node->refType == GlrRefType::Id)
						{
							TraverseRule(node->literal);
						}
					}

					void Traverse(GlrUseSyntax* node) override
					{
						TraverseRule(node->name);
					}

					void Visit(GlrTestConditionSyntax* node) override
					{
					}

					void Visit(GlrPushConditionSyntax* node) override
					{
					}

				public:
					ExpandSwitchSyntaxVisitor(
						VisitorContext& _vContext,
						RewritingContext& _rContext
					)
						: vContext(_vContext)
						, rContext(_rContext)
					{
					}

					void InspectIntoAffectedRule(GlrRule* rule, Ptr<Dictionary<WString, bool>> currentSwitchValues)
					{
					}

					void InspectIntoUnaffectedRule(GlrRule* rule)
					{
						auto workingRule = vContext.syntaxManager.Rules()[rule->name.value];
						if (scannedUnaffectedRules.Contains(workingRule)) return;
						scannedUnaffectedRules.Add(workingRule);

						auto oldId = identification;
						identification = {};
						identification.workingRule = workingRule;
						InspectInto(rule);
						identification = oldId;
					}
				};
			}

/***********************************************************************
CopyAffectedRuleVisitor
***********************************************************************/

/***********************************************************************
CopyUnaffectedRuleVisitor
***********************************************************************/

/***********************************************************************
RewriteSyntax
***********************************************************************/

			Ptr<GlrSyntaxFile> RewriteSyntax_Switch(VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				using namespace rewritesyntax_switch;

				// merge files to single syntax file
				auto rewritten = MakePtr<GlrSyntaxFile>();
				for (auto file : files)
				{
					CopyFrom(rewritten->rules, file->rules, true);
				}

				RewritingContext rewritingContext;
				{
					ExpandSwitchSyntaxVisitor visitor(context, rewritingContext);
					for (auto file : files)
					{
						for (auto rule : file->rules)
						{
							auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
							if (!context.ruleAffectedSwitches.Keys().Contains(ruleSymbol))
							{
								visitor.InspectIntoUnaffectedRule(rule.Obj());
							}
						}
					}
				}

				syntaxManager.ClearSwitches();
				return rewritten;
			}
		}
	}
}