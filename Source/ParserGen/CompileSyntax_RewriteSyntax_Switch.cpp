#include "Compiler.h"
#include "../ParserGen_Generated/ParserGenRuleAst_Traverse.h"
#include "../ParserGen_Generated/ParserGenRuleAst_Copy.h"

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
					GlrRule*									expandedRule = nullptr;
					Ptr<Dictionary<WString, bool>>				switchValues;
				};

				struct RewritingContext
				{
					Group<RuleSymbol*, Ptr<GeneratedRule>>		generatedRules;
				};

				class EmptySyntax : public GlrSyntax
				{
				public:
					void Accept(GlrSyntax::IVisitor* visitor) override
					{
						CHECK_FAIL(L"vl::glr::parsergen::rewritesyntax_switch::EmptySyntax::Accept(GlrSyntax::IVisitor*)#EmptySyntax should not be used!");
					}
				};

				Ptr<Dictionary<WString, bool>> ApplySwitches(Ptr<Dictionary<WString, bool>> currentValues, GlrPushConditionSyntax* node)
				{
					auto newValues = MakePtr<Dictionary<WString, bool>>();
					if (currentValues)
					{
						CopyFrom(*newValues.Obj(), *currentValues.Obj());
					}
					for (auto switchItem : node->switches)
					{
						newValues->Set(switchItem->name.value, switchItem->value == GlrSwitchValue::True);
					}
					return newValues;
				}

/***********************************************************************
EvaluateConditionVisitor
**********************************************************************/

				class EvaluateConditionVisitor
					: public Object
					, protected GlrCondition::IVisitor
				{
					// this visitor evaluates a condition with given switch values
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
					// this visitor look into all rules which is affected by switch values
					// and find all possible switch combination for these rules
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
						// check if it is a rule
						vint index = vContext.syntaxManager.Rules().Keys().IndexOf(ruleName.value);
						if (index == -1) return;

						auto ruleSymbol = vContext.syntaxManager.Rules().Values()[index];
						auto rule = vContext.astRules[ruleSymbol];

						// check if it is affected
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
							// inspect into the branch only when it satisfies the condition
							if (!branch || visitor.Evaluate(branch->condition.Obj()))
							{
								InspectInto(branch->syntax.Obj());
							}
						}
					}

					void Visit(GlrPushConditionSyntax* node) override
					{
						// inspect into the syntax with updated values
						auto oldValues = identification.workingSwitchValues;
						auto newValues = ApplySwitches(oldValues, node);

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
						// an affected rule respond to switch values
						// collect switch values that the rule cares
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

						// skip if the rule with collected switch values has been inspected
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

						// make a record of the collected switch values
						auto generatedRule = MakePtr<GeneratedRule>();
						generatedRule->ruleToExpand = rule;
						generatedRule->switchValues = workingSwitchValues;
						rContext.generatedRules.Add(workingRule, generatedRule);

						// inspect into the rule with collected switch values
						auto oldId = identification;
						identification.workingRule = workingRule;
						identification.workingSwitchValues = workingSwitchValues;
						identification.generatedRule = generatedRule;
						InspectInto(rule);
						identification = oldId;
					}

					void InspectIntoUnaffectedRule(GlrRule* rule)
					{
						// an unaffected rule does not respond to switch values
						// skip if it has been inspected
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

/***********************************************************************
ExpandClauseVisitor
***********************************************************************/

				class ExpandClauseVisitor : public copy_visitor::RuleAstVisitor
				{
					// this visitor expand syntax with given switch values
					// the expanded syntax does not include GlrPushConditionSyntax
					// GlrTestConditionSyntax will be possibly converts to GlrAlternativeSyntax
					//   only with branches whose condition is evaluated to true under given switch values
					// EmptySyntax will be generated for ?(true: ; | false: allOtherBranches)
					// CancelBranch will be thrown for ?(false: allBranches)
				protected:
					struct CancelBranch {};

					VisitorContext&						vContext;
					Ptr<Dictionary<WString, bool>>		workingSwitchValues;

				protected:

					void FixRuleName(ParsingToken& name)
					{
						// check if it is an affected rule
						vint index = vContext.syntaxManager.Rules().Keys().IndexOf(name.value);
						if (index == -1) return;

						auto ruleSymbol = vContext.syntaxManager.Rules().Values()[index];
						index = vContext.ruleAffectedSwitches.Keys().IndexOf(ruleSymbol);
						if (index == -1) return;

						// for an affected rule
						// the name referencing the rule need to be changed
						// by appending switch values after it
						SortedList<WString> switchNames;
						CopyFrom(switchNames, vContext.ruleAffectedSwitches.GetByIndex(index));

						name.value += L"_SWITCH";
						for (auto&& switchName : switchNames)
						{
							auto value = workingSwitchValues->Get(switchName);
							name.value += (value ? L"_1" : L"_0") + switchName;
						}
					}

					void ExpandSyntaxToList(Ptr<GlrSyntax> syntax, List<Ptr<GlrSyntax>>& items)
					{
						try
						{
							items.Add(CopyNode(syntax.Obj()));
						}
						catch (CancelBranch)
						{
						}
					}

					void BuildAlt(bool optional, List<Ptr<GlrSyntax>>& items)
					{
						// build syntax [items...]
						if (items.Count() == 0)
						{
							// if there is no branch
							if (optional)
							{
								// optional of nothing is EmptySyntax
								result = MakePtr<EmptySyntax>();
								return;
							}
							else
							{
								// non-optional of nothing results in an exception
								result = nullptr;
								throw CancelBranch();
							}
						}
						else if (items.Count() == 1)
						{
							// if there is only on branch, just use it
							result = items[0];
						}
						else
						{
							// otherwise create alternative syntax for them
							auto alt = MakePtr<GlrAlternativeSyntax>();
							alt->first = items[0];
							alt->second = items[1];
							for (vint i = 2; i < items.Count(); i++)
							{
								auto newAlt = MakePtr<GlrAlternativeSyntax>();
								newAlt->first = alt;
								newAlt->second = items[i];
								alt = newAlt;
							}
							result = alt;
						}

						if (optional)
						{
							// make it optional if necessary
							auto opt = MakePtr<GlrOptionalSyntax>();
							opt->syntax = result.Cast<GlrSyntax>();
							result = opt;
						}
					}

				protected:

					void Visit(GlrRefSyntax* node) override
					{
						// only need to fix rule name for GlrRefSyntax
						copy_visitor::RuleAstVisitor::Visit(node);
						if (node->refType == GlrRefType::Id)
						{
							FixRuleName(result.Cast<GlrRefSyntax>()->literal);
						}
					}

					void Visit(GlrUseSyntax* node) override
					{
						// only need to fix rule name for GlrUseSyntax
						copy_visitor::RuleAstVisitor::Visit(node);
						FixRuleName(result.Cast<GlrUseSyntax>()->name);
					}

					void Visit(GlrAlternativeSyntax* node) override
					{
						// alternative syntax converts to alternative syntax
						List<Ptr<GlrSyntax>> items;
						ExpandSyntaxToList(node->first, items);
						ExpandSyntaxToList(node->second, items);
						BuildAlt(false, items);
					}

					void Visit(GlrTestConditionSyntax* node) override
					{
						// test condition syntax converts alternative syntax with optional if necessary
						bool optional = false;
						List<Ptr<GlrSyntax>> items;
						EvaluateConditionVisitor visitor(*workingSwitchValues.Obj());
						for (auto branch : node->branches)
						{
							if (!branch || visitor.Evaluate(branch->condition.Obj()))
							{
								if (branch->syntax)
								{
									ExpandSyntaxToList(branch->syntax, items);
								}
								else
								{
									optional = true;
								}
							}
						}
						BuildAlt(optional, items);
					}

					void Visit(GlrPushConditionSyntax* node) override
					{
						// push condition syntax will be replaced by the syntax inside it
						auto oldValues = workingSwitchValues;
						auto newValues = ApplySwitches(oldValues, node);

						workingSwitchValues = newValues;
						node->syntax->Accept(this);
						workingSwitchValues = oldValues;
					}

				public:
					ExpandClauseVisitor(
						VisitorContext& _vContext,
						Ptr<Dictionary<WString, bool>> _workingSwitchValues
					)
						: vContext(_vContext)
						, workingSwitchValues(_workingSwitchValues)
					{
					}

					Ptr<GlrClause> ExpandClause(GlrClause* node)
					{
						try
						{
							return CopyNode(node);
						}
						catch (CancelBranch)
						{
							return nullptr;
						}
					}
				};

/***********************************************************************
DeductEmptySyntaxVisitor
***********************************************************************/

				class DeductEmptySyntaxVisitor : public copy_visitor::RuleAstVisitor
				{
					// this visitor rewrite a syntax by removing EmptySyntax
					// but it is possible that the whole syntax is evaluated to EmptySyntax
					// in this case it is rewritten to EmptySyntax
				protected:

					Ptr<GlrSyntax> CopyNodeSafe(Ptr<GlrSyntax> node)
					{
						if (node.Cast<EmptySyntax>())
						{
							return node;
						}
						else
						{
							return CopyNode(node.Obj());
						}
					}
				protected:
					void Visit(GlrRefSyntax* node) override
					{
						result = node;
					}

					void Visit(GlrUseSyntax* node) override
					{
						result = node;
					}

					void Visit(GlrLoopSyntax* node) override
					{
						node->syntax = CopyNodeSafe(node->syntax);
						node->delimiter = CopyNodeSafe(node->delimiter);
						result = node;

						if (node->syntax.Cast<EmptySyntax>())
						{
							// if the loop body is empty, it is empty
							result = node->syntax;
						}
						else if (node->delimiter.Cast<EmptySyntax>())
						{
							node->delimiter = nullptr;
						}
					}

					void Visit(GlrOptionalSyntax* node) override
					{
						node->syntax = CopyNodeSafe(node->syntax);
						result = node;

						if (node->syntax.Cast<EmptySyntax>())
						{
							// if the optional body is empty, it is empty
							result = node->syntax;
						}
					}

					void Visit(GlrSequenceSyntax* node) override
					{
						node->first = CopyNodeSafe(node->first);
						node->second = CopyNodeSafe(node->second);
						result = node;

						bool first = !node->first.Cast<EmptySyntax>();
						bool second = !node->second.Cast<EmptySyntax>();
						if (first && second)
						{
							// if both are not empty, nothing need to worry
						}
						else if (first)
						{
							// if only first is not empty, it is second
							result = node->first;
						}
						else if (second)
						{
							// if only second is empty, it is second
							result = node->second;
						}
						else
						{
							// if both are empty, it is empty
							result = node->first;
						}
					}

					void Visit(GlrAlternativeSyntax* node) override
					{
						node->first = CopyNodeSafe(node->first);
						node->second = CopyNodeSafe(node->second);
						result = node;

						bool first = !node->first.Cast<EmptySyntax>();
						bool second = !node->second.Cast<EmptySyntax>();
						if (first && second)
						{
							// if both are not empty, nothing need to worry
						}
						else if (first)
						{
							// if only first is not empty, it is [first]
							auto opt = MakePtr<GlrOptionalSyntax>();
							opt->syntax = node->first;
							result = opt;
						}
						else if (second)
						{
							// if only second is not empty, it is [second]
							auto opt = MakePtr<GlrOptionalSyntax>();
							opt->syntax = node->second;
							result = opt;
						}
						else
						{
							// if both are empty, it is empty
							result = node->first;
						}
					}

					void Visit(GlrPushConditionSyntax* node) override
					{
						CHECK_FAIL(L"vl::glr::parsergen::rewritesyntax_switch::DeductEmptySyntaxVisitor::Visit(GlrPushConditionSyntax*)#This should have been removed.");
					}

					void Visit(GlrTestConditionSyntax* node) override
					{
						CHECK_FAIL(L"vl::glr::parsergen::rewritesyntax_switch::DeductEmptySyntaxVisitor::Visit(GlrTestConditionSyntax*)#This should have been removed.");
					}
				};

/***********************************************************************
DeductAndVerifyClauseVisitor
**********************************************************************/

				class DeductAndVerifyClauseVisitor
					: public Object
					, protected GlrClause::IVisitor
				{
					// this visitor remove EmptySyntax in clauses
					// if it is not possible, it returns false
				protected:
					void Verify(Ptr<GlrSyntax>& syntax)
					{
						auto deducted = DeductEmptySyntaxVisitor().CopyNode(syntax.Obj());
						if (deducted.Cast<EmptySyntax>())
						{
							result = false;
						}
						else
						{
							syntax = deducted;
							result = true;
						}
					}

					void Visit(GlrCreateClause* node) override
					{
						Verify(node->syntax);
					}

					void Visit(GlrPartialClause* node) override
					{
						Verify(node->syntax);
					}

					void Visit(GlrReuseClause* node) override
					{
						Verify(node->syntax);
					}

					void Visit(GlrLeftRecursionPlaceholderClause* node) override
					{
						result = true;
					}

					void Visit(GlrLeftRecursionInjectClause* node) override
					{
						result = true;
					}

					void Visit(GlrPrefixMergeClause* node) override
					{
						result = true;
					}

				public:
					bool								result = false;

					bool Evaluate(GlrClause* node)
					{
						node->Accept(this);
						return result;
					}
				};
			}

/***********************************************************************
RewriteSyntax
***********************************************************************/

			Ptr<GlrSyntaxFile> RewriteSyntax_Switch(VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				using namespace rewritesyntax_switch;

				if (context.ruleAffectedSwitches.Count() == syntaxManager.Rules().Count())
				{
					syntaxManager.AddError(
						ParserErrorType::NoSwitchUnaffectedRule,
						{}
						);
				}

				RewritingContext rewritingContext;
				{
					// find out all expansion of rules affected by switch values
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

				auto rewritten = MakePtr<GlrSyntaxFile>();
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
						vint index = rewritingContext.generatedRules.Keys().IndexOf(ruleSymbol);
						if (index == -1)
						{
							// if a rule is unaffected
							// just remove GlrPushConditionSyntax in clauses
							// rules it references could be renamed
							auto newRule = MakePtr<GlrRule>();
							newRule->codeRange = rule->codeRange;
							newRule->name = rule->name;
							newRule->type = rule->type;

							ExpandClauseVisitor visitor(context, nullptr);
							for (auto clause : rule->clauses)
							{
								if (auto newClause = visitor.ExpandClause(clause.Obj()))
								{
									newRule->clauses.Add(newClause);
								}
							}

							if (newRule->clauses.Count() == 0)
							{
								// a rewritten rule must have at least on clause
								syntaxManager.AddError(
									ParserErrorType::SwitchUnaffectedRuleExpandedToNoClause,
									rule->codeRange,
									rule->name.value
									);
							}
							else
							{
								rewritten->rules.Add(newRule);
							}
						}
						else
						{
							// if a rule is affected
							// all instances of possible switch values will be converted to a new rule
							// such rule has switch values encoded in its name
							for(auto generatedRule : From(rewritingContext.generatedRules.GetByIndex(index))
								.OrderBy([](Ptr<GeneratedRule> a, Ptr<GeneratedRule> b)
								{
									return CompareEnumerable(a->switchValues->Values(), b->switchValues->Values());
								}))
							{
								auto newRule = MakePtr<GlrRule>();
								newRule->codeRange = rule->codeRange;
								newRule->name = rule->name;
								newRule->type = rule->type;

								newRule->name.value += L"_SWITCH";
								for (auto [name, value] : *generatedRule->switchValues.Obj())
								{
									newRule->name.value += (value ? L"_1" : L"_0") + name;
								}

								generatedRule->expandedRule = newRule.Obj();

								// rewrite all clauses with given switch values
								ExpandClauseVisitor visitor(context, generatedRule->switchValues);
								for (auto clause : rule->clauses)
								{
									if (auto newClause = visitor.ExpandClause(clause.Obj()))
									{
										if (DeductAndVerifyClauseVisitor().Evaluate(newClause.Obj()))
										{
											// only add the rewritten clause to the generated rule if it is valid
											// a clause could be invalid if all branches evaluated to nothing due to GlrTestConditionSyntax
											newRule->clauses.Add(newClause);
										}
									}
								}

								if (newRule->clauses.Count() == 0)
								{
									// a rewritten rule must have at least on clause
									syntaxManager.AddError(
										ParserErrorType::SwitchAffectedRuleExpandedToNoClause,
										rule->codeRange,
										rule->name.value,
										newRule->name.value
										);
								}
								else
								{
									rewritten->rules.Add(newRule);
								}
							}
						}
					}
				}

				// add symbols for generated rules
				for (auto [ruleSymbol, index] : indexed(rewritingContext.generatedRules.Keys()))
				{
					for (auto generatedRule : rewritingContext.generatedRules.GetByIndex(index))
					{
						auto newRuleSymbol = syntaxManager.CreateRule(
							generatedRule->expandedRule->name.value,
							generatedRule->expandedRule->name.codeRange
							);
						newRuleSymbol->ruleType = ruleSymbol->ruleType;
					}
				}

				// remove symbols for rule affected by switch values
				// because they are expanded to other rules
				for (auto ruleSymbol : rewritingContext.generatedRules.Keys())
				{
					syntaxManager.RemoveRule(ruleSymbol->Name());
				}

				return rewritten;
			}
		}
	}
}