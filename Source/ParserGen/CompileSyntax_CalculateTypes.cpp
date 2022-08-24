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
ValidatePartialRules
***********************************************************************/

			void ValidatePartialRules(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						List<Ptr<GlrPartialClause>> partialClauses;
						CopyFrom(partialClauses, From(rule->clauses).FindType<GlrPartialClause>());
						if (partialClauses.Count() == 0) continue;

						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						ruleSymbol->isPartial = partialClauses.Count() > 0;

						if (partialClauses.Count() != rule->clauses.Count())
						{
							context.syntaxManager.AddError(
								ParserErrorType::RuleMixedPartialClauseWithOtherClause,
								rule->codeRange,
								ruleSymbol->Name()
								);
						}

						AstClassSymbol* partialType = nullptr;
						for (auto clause : partialClauses)
						{
							vint index = context.clauseTypes.Keys().IndexOf(clause.Obj());
							if (index != -1)
							{
								auto type = context.clauseTypes.Values()[index];
								if (!partialType)
								{
									partialType = type;
								}
								else if (type && partialType != type)
								{
									context.syntaxManager.AddError(
										ParserErrorType::RuleWithDifferentPartialTypes,
										rule->codeRange,
										ruleSymbol->Name(),
										partialType->Name(),
										type->Name()
										);
									break;
								}
							}
						}
					}
				}
			}

/***********************************************************************
CalculateRuleAndClauseTypes
***********************************************************************/

			WString GetRuleTypes(const IEnumerable<RuleSymbol*>& rules)
			{
				return
					From(rules)
					.Select([](auto r) { return r->ruleType; })
					.Where([](auto rt) { return rt != nullptr; })
					.Select([](auto rt) { return rt->Name(); })
					.Aggregate(WString::Empty, [](auto a, auto b) {return a == WString::Empty ? b : a + L", " + b; })
					;
			}

			void CalculateRuleAndClauseTypes(VisitorContext& context)
			{
				// find cyclic dependencies in "Rule ::= !Rule"
				auto&& rules = context.syntaxManager.Rules().Values();
				PartialOrderingProcessor pop;
				pop.InitWithGroup(rules, context.ruleReuseDependencies);
				pop.Sort();

				// remove cyclic dependended rules from ruleReuseDependencies
				List<List<RuleSymbol*>> cyclicReuseDependencies;
				for (auto&& component : pop.components)
				{
					if (component.nodeCount == 1) continue;

					for (vint i = 0; i < component.nodeCount - 1; i++)
					{
						for (vint j = i + 1; j < component.nodeCount; j++)
						{
							auto r1 = rules[component.firstNode[i]];
							auto r2 = rules[component.firstNode[j]];
							context.ruleReuseDependencies.Remove(r1, r2);
							context.ruleReuseDependencies.Remove(r2, r1);
						}
					}

					List<RuleSymbol*> cyclicRules;
					for (vint i = 0; i < component.nodeCount; i++)
					{
						cyclicRules.Add(rules[component.firstNode[i]]);
					}
					cyclicReuseDependencies.Add(std::move(cyclicRules));
				}

				// do not update explicitly specified rule type
				SortedList<RuleSymbol*> explicitlyTypedRules;
				for (auto rule : rules)
				{
					if (rule->ruleType)
					{
						explicitlyTypedRules.Add(rule);
					}
				}

				// define updateRuleType function, check clause type added to rule
				auto updateRuleType = [&context, &explicitlyTypedRules](RuleSymbol* rule, AstClassSymbol* newClauseType, bool promptIfNull)
				{
					auto newRuleType = FindCommonBaseClass(rule->ruleType, newClauseType);
					if (explicitlyTypedRules.Contains(rule))
					{
						if (rule->ruleType != newRuleType)
						{
							context.syntaxManager.AddError(
								ParserErrorType::RuleExplicitTypeIsNotCompatibleWithClauseType,
								context.astRules[rule]->codeRange,
								rule->Name(),
								(rule->ruleType ? rule->ruleType->Name() : WString::Empty),
								(newClauseType ? newClauseType->Name() : WString::Empty)
								);
							return false;
						}
					}
					else
					{
						if (promptIfNull && !newRuleType)
						{
							context.syntaxManager.AddError(
								ParserErrorType::RuleCannotResolveToDeterministicType,
								context.astRules[rule]->codeRange,
								rule->Name(),
								(rule->ruleType ? rule->ruleType->Name() : WString::Empty),
								(newClauseType ? newClauseType->Name() : WString::Empty)
								);
							return false;
						}
						rule->ruleType = newRuleType;
					}
					return true;
				};

				// calculate types for rules from clauses with known types
				for (auto rule : rules)
				{
					for (auto clause : context.astRules[rule]->clauses)
					{
						vint index = context.clauseTypes.Keys().IndexOf(clause.Obj());
						if (index != -1)
						{
							auto newClauseType = context.clauseTypes.Values()[index];
							if (!updateRuleType(rule, newClauseType, true))
							{
								break;
							}
						}
					}
				}
				if (context.global.Errors().Count() > 0) return;

				// calculate types for rules that contain reuse dependency
				for (auto&& component : pop.components)
				{
					for (vint i = 0; i < component.nodeCount; i++)
					{
						auto rule = rules[component.firstNode[i]];
						vint index = context.ruleReuseDependencies.Keys().IndexOf(rule);
						if (index != -1)
						{
							AstClassSymbol* type = nullptr;
							for (auto dep : context.ruleReuseDependencies.GetByIndex(index))
							{
								type = FindCommonBaseClass(type, dep->ruleType);
							}
							if (type && !updateRuleType(rule, type, true))
							{
								break;
							}
						}
					}
				}
				if (context.global.Errors().Count() > 0) return;

				// calculate types for rules that contain cyclic reuse dependency
				for (auto&& cyclicRules : cyclicReuseDependencies)
				{
					AstClassSymbol* type = nullptr;
					for (auto rule : cyclicRules)
					{
						type = FindCommonBaseClass(type, rule->ruleType);
					}

					if (!type)
					{
						auto ruleTypes = GetRuleTypes(cyclicRules);
						for (auto rule : cyclicRules)
						{
							context.syntaxManager.AddError(
								ParserErrorType::CyclicDependedRuleTypeIncompatible,
								context.astRules[rule]->codeRange,
								rule->Name(),
								ruleTypes
								);
						}
					}
					else
					{
						for (auto rule : cyclicRules)
						{
							updateRuleType(rule, type, false);
						}
					}
				}

				// prompt errors
				for (auto rule : rules)
				{
					if (!rule->ruleType)
					{
						context.syntaxManager.AddError(
							ParserErrorType::RuleCannotResolveToDeterministicType,
							context.astRules[rule]->codeRange,
							rule->Name()
							);
					}
				}

				// calculate types for reuse clauses
				for (auto astRule : context.astRules.Values())
				{
					for (auto clause : From(astRule->clauses).FindType<GlrReuseClause>())
					{
						vint index = context.clauseReuseDependencies.Keys().IndexOf(clause.Obj());
						if (index == -1)
						{
							context.syntaxManager.AddError(
								ParserErrorType::ReuseClauseContainsNoUseRule,
								clause->codeRange,
								astRule->name.value
								);
						}
						else
						{
							AstClassSymbol* type = nullptr;
							for (auto dep : context.clauseReuseDependencies.GetByIndex(index))
							{
								type = FindCommonBaseClass(type, dep->ruleType);
							}

							if (type)
							{
								context.clauseTypes.Add(clause.Obj(), type);
							}
							else
							{
								auto ruleTypes = GetRuleTypes(context.clauseReuseDependencies.GetByIndex(index));
								context.syntaxManager.AddError(
									ParserErrorType::ReuseClauseCannotResolveToDeterministicType,
									clause->codeRange,
									astRule->name.value,
									ruleTypes
									);
							}
						}
					}
				}
			}
		}
	}
}