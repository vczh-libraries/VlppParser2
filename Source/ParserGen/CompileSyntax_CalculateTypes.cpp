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
							context.syntaxManager.Global().AddError(
								ParserErrorType::RuleMixedPartialClauseWithOtherClause,
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
									context.syntaxManager.Global().AddError(
										ParserErrorType::RuleWithDifferentPartialTypes,
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

			AstClassSymbol* MergeClassSymbol(AstClassSymbol* c1, AstClassSymbol* c2)
			{
				if (c1 == c2) return c1;
				if (!c1) return c2;
				if (!c2) return c1;

				// find common base classes
				vint n1 = 0, n2 = 0;
				{
					auto c = c1;
					while (c)
					{
						n1++;
						c = c->baseClass;
					}
				}
				{
					auto c = c2;
					while (c)
					{
						n2++;
						c = c->baseClass;
					}
				}

				while (n1 > n2)
				{
					n1--;
					c1 = c1->baseClass;
				}
				while (n2 > n1)
				{
					n2--;
					c2 = c2->baseClass;
				}

				while (c1 && c2)
				{
					if (c1 == c2) return c1;
					c1 = c1->baseClass;
					c2 = c2->baseClass;
				}
				return nullptr;
			}

			void CalculateRuleAndClauseTypes(VisitorContext& context)
			{
				// find cyclic dependencies in "Rule ::= !Rule"
				auto&& rules = context.syntaxManager.Rules().Values();
				PartialOrderingProcessor pop;
				pop.InitWithGroup(rules, context.ruleReuseDependencies);
				pop.Sort();

				// remove cyclic dependended rules from ruleReuseDependencies
				for (auto&& component : pop.components)
				{
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
				}

				// calculate types for rules from clauses with known types
				for (auto rule : rules)
				{
					for (auto clause : context.astRules[rule]->clauses)
					{
						vint index = context.clauseTypes.Keys().IndexOf(clause.Obj());
						if (index != -1)
						{
							rule->ruleType = MergeClassSymbol(rule->ruleType, context.clauseTypes.Values()[index]);
							if (!rule->ruleType)
							{
								context.global.AddError(
									ParserErrorType::RuleCannotResolveToDeterministicType,
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
								type = MergeClassSymbol(type, dep->ruleType);
							}
							if (type)
							{
								rule->ruleType = MergeClassSymbol(rule->ruleType, type);
								if (!rule->ruleType)
								{
									context.global.AddError(
										ParserErrorType::RuleCannotResolveToDeterministicType,
										rule->Name()
									);
									break;
								}
							}
						}
					}
				}
				if (context.global.Errors().Count() > 0) return;

				// prompt errors
				for (auto rule : rules)
				{
					if (!rule->ruleType)
					{
						context.global.AddError(
							ParserErrorType::RuleCannotResolveToDeterministicType,
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
							context.global.AddError(
								ParserErrorType::ReuseClauseContainsNoUseRule,
								astRule->name.value
								);
						}
						else
						{
							AstClassSymbol* type = nullptr;
							for (auto dep : context.clauseReuseDependencies.GetByIndex(index))
							{
								type = MergeClassSymbol(type, dep->ruleType);
							}

							if (type)
							{
								context.clauseTypes.Add(clause.Obj(), type);
							}
							else
							{
								context.global.AddError(
									ParserErrorType::ReuseClauseCannotResolveToDeterministicType,
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