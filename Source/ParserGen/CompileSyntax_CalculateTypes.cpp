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
										ruleSymbol->Name()
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

				// calculate types for rules from clauses with known types
				for (auto rule : rules)
				{
					for (auto clause : context.astRules[rule]->clauses)
					{
						vint index = context.clauseTypes.Keys().IndexOf(clause.Obj());
						if (index != -1)
						{
							rule->ruleType = FindCommonBaseClass(rule->ruleType, context.clauseTypes.Values()[index]);
							if (!rule->ruleType)
							{
								context.syntaxManager.AddError(
									ParserErrorType::RuleCannotResolveToDeterministicType,
									context.astRules[rule]->codeRange,
									rule->Name()
									);
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
							if (type)
							{
								rule->ruleType = FindCommonBaseClass(rule->ruleType, type);
								if (!rule->ruleType)
								{
									context.syntaxManager.AddError(
										ParserErrorType::RuleCannotResolveToDeterministicType,
										context.astRules[rule]->codeRange,
										rule->Name()
										);
									break;
								}
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
						for (auto rule : cyclicRules)
						{
							context.syntaxManager.AddError(
								ParserErrorType::CyclicDependedRuleTypeIncompatible,
								context.astRules[rule]->codeRange,
								rule->Name()
								);
						}
					}
					else
					{
						for (auto rule : cyclicRules)
						{
							rule->ruleType = type;
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
								context.syntaxManager.AddError(
									ParserErrorType::ReuseClauseCannotResolveToDeterministicType,
									clause->codeRange,
									astRule->name.value
									);
							}
						}
					}
				}
			}
		}
	}
}