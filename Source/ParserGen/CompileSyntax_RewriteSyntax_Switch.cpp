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
					GlrRule*									ruleToExpand = nullptr;
					Ptr<Dictionary<WString, bool>>				switchValues;
				};

				struct RewritingContext
				{
					Group<RuleSymbol*, Ptr<GeneratedRule>>		generatedRules;
				};

/***********************************************************************
EvaluateConditionVisitor
**********************************************************************/

				class EvaluateConditionVisitor
					: public Object
					, protected GlrCondition::IVisitor
				{
				protected:
					Dictionary<WString, bool>&			workingSwitchValues;

					void Visit(GlrRefCondition* node) override
					{
						vint index = workingSwitchValues.Keys().IndexOf(node->name.value);
						CHECK_ERROR(
							index != -1,
							L"vl::glr::parsergen::rewritesyntax_switch::EvaluateConditionVisitor::Visit(GlrRefCondition*)#"
							L"Internal error: specified switch value is unevaluated"
						);
						result = workingSwitchValues.Values()[index];
					}

					void Visit(GlrNotCondition* node) override
					{
						node->condition->Accept(this);
						result = !result;
					}

					void Visit(GlrAndCondition* node) override
					{
						node->first->Accept(this);
						bool first = result;
						node->second->Accept(this);
						bool second = result;
						result = first && second;
					}

					void Visit(GlrOrCondition* node) override
					{
						node->first->Accept(this);
						bool first = result;
						node->second->Accept(this);
						bool second = result;
						result = first || second;
					}

				public:
					bool								result = false;

					EvaluateConditionVisitor(
						Dictionary<WString, bool>& _workingSwitchValues
					)
						: workingSwitchValues(_workingSwitchValues)
					{
					}

					bool Evaluate(GlrCondition* condition)
					{
						condition->Accept(this);
						return result;
					}
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
							InspectIntoAffectedRule(
								rule,
								vContext.ruleAffectedSwitches.GetByIndex(index),
								identification.workingSwitchValues
							);
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
						CHECK_ERROR(
							identification.workingSwitchValues,
							L"vl::glr::parsergen::rewritesyntax_switch::ExpandSwitchSyntaxVisitor::Visit(GlrTestConditionSyntax*)#"
							L"Internal error: switch values are unevaluated"
						);
						EvaluateConditionVisitor visitor(*identification.workingSwitchValues.Obj());

						for (auto branch : node->branches)
						{
							if (!branch || visitor.Evaluate(branch->condition.Obj()))
							{
								InspectInto(branch->syntax.Obj());
							}
						}
					}

					void Visit(GlrPushConditionSyntax* node) override
					{
						auto newValues = MakePtr<Dictionary<WString, bool>>();
						if (identification.workingSwitchValues)
						{
							CopyFrom(*newValues.Obj(), *identification.workingSwitchValues.Obj());
						}
						for (auto switchItem : node->switches)
						{
							newValues->Set(switchItem->name.value, switchItem->value == GlrSwitchValue::True);
						}

						auto oldValues = identification.workingSwitchValues;
						identification.workingSwitchValues = newValues;
						traverse_visitor::RuleAstVisitor::Visit(node);
						identification.workingSwitchValues = oldValues;
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

					void InspectIntoAffectedRule(
						GlrRule* rule,
						const List<WString>& affectedSwitches,
						Ptr<Dictionary<WString, bool>> currentSwitchValues
					)
					{
						auto workingSwitchValues = MakePtr<Dictionary<WString, bool>>();
						for (auto&& name : affectedSwitches)
						{
							vint index = -1;
							if (currentSwitchValues)
							{
								index = currentSwitchValues->Keys().IndexOf(name);
							}

							if (index == -1)
							{
								workingSwitchValues->Set(name, vContext.syntaxManager.switches[name].defaultValue);
							}
							else
							{
								workingSwitchValues->Set(name, currentSwitchValues->Values()[index]);
							}
						}

						auto workingRule = vContext.syntaxManager.Rules()[rule->name.value];
						vint index = rContext.generatedRules.Keys().IndexOf(workingRule);
						if (index != -1)
						{
							for (auto generatedRule : rContext.generatedRules.GetByIndex(index))
							{
								if (CompareEnumerable(*generatedRule->switchValues.Obj(), *workingSwitchValues.Obj()) == 0)
								{
									return;
								}
							}
						}

						auto generatedRule = MakePtr<GeneratedRule>();
						generatedRule->ruleToExpand = rule;
						generatedRule->switchValues = workingSwitchValues;
						rContext.generatedRules.Add(workingRule, generatedRule);

						auto oldId = identification;
						identification.workingRule = workingRule;
						identification.workingSwitchValues = workingSwitchValues;
						identification.generatedRule = generatedRule;
						InspectInto(rule);
						identification = oldId;
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

				CHECK_FAIL(L"RewriteSyntax_Switch Not Implemented!");
				syntaxManager.ClearSwitches();
				auto rewritten = MakePtr<GlrSyntaxFile>();
				return rewritten;
			}
		}
	}
}