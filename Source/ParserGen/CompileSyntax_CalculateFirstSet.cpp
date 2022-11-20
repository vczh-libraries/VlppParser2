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
DirectFirstSetVisitor
***********************************************************************/

			class DirectFirstSetVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			private:
				bool						couldBeEmpty = false;

			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					currentClause = nullptr;

				RuleSymbol* TryGetRuleSymbol(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index == -1) return nullptr;
					return context.syntaxManager.Rules().Values()[index];
				}

				void AddStartRule(const WString& name)
				{
					if (auto startRule = TryGetRuleSymbol(name))
					{
						context.directStartRules.Add(ruleSymbol, { startRule,currentClause });
						context.clauseToStartRules.Add(currentClause, ruleSymbol);
						if (ruleSymbol == startRule && !context.leftRecursiveClauses.Contains(ruleSymbol, currentClause))
						{
							context.leftRecursiveClauses.Add(ruleSymbol, currentClause);
						}
					}
				}

			public:
				DirectFirstSetVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void VisitClause(Ptr<GlrClause> clause)
				{
					currentClause = clause.Obj();
					clause->Accept(this);
				}
			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					if (node->refType == GlrRefType::Id)
					{
						AddStartRule(node->literal.value);
					}
					couldBeEmpty = false;
				}

				void Visit(GlrUseSyntax* node) override
				{
					AddStartRule(node->name.value);
					couldBeEmpty = false;
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					couldBeEmpty = true;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
					couldBeEmpty = true;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					if (couldBeEmpty) node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					bool firstCouldBeEmpty = couldBeEmpty;
					node->second->Accept(this);
					bool secondCouldBeEmpty = couldBeEmpty;
					couldBeEmpty = firstCouldBeEmpty || secondCouldBeEmpty;
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					CHECK_FAIL(L"GlrPushConditionSyntax should have been removed after RewriteSyntax_Switch()!");
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					CHECK_FAIL(L"GlrTestConditionSyntax should have been removed after RewriteSyntax_Switch()!");
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
					if (node->assignments.Count() == 0)
					{
						auto nodeSyntax = node->syntax.Obj();
						auto pushSyntax = dynamic_cast<GlrPushConditionSyntax*>(nodeSyntax);
						if (pushSyntax) nodeSyntax = pushSyntax->syntax.Obj();

						if (auto useSyntax = dynamic_cast<GlrUseSyntax*>(nodeSyntax))
						{
							if (auto startRule = TryGetRuleSymbol(useSyntax->name.value))
							{
								context.directSimpleUseRules.Add(ruleSymbol, { startRule,currentClause });
								context.simpleUseClauseToReferencedRules.Add(currentClause, startRule);
							}
						}
					}
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					node->rule->Accept(this);
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
					node->rule->Accept(this);
				}
			};

/***********************************************************************
CalculateFirstSet
***********************************************************************/

			void CalculateFirstSet_DirectStartRules(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
				{
					auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					DirectFirstSetVisitor visitor(context, ruleSymbol);
					for (auto clause : rule->clauses)
					{
						visitor.VisitClause(clause);
					}
				}
			}

			void CalculateFirstSet_RuleClosure(const RulePathDependencies& direct, RulePathDependencies& indirect, PathToLastRuleMap& pathToLastRules)
			{
				for (auto [rule, index] : indexed(direct.Keys()))
				{
					auto&& startRules = direct.GetByIndex(index);
					for (auto [startRule, clause] : startRules)
					{
						indirect.Add(rule, { startRule,clause });
						pathToLastRules.Add({ rule,startRule }, { rule,clause });
					}
				}

				while (true)
				{
					vint offset = 0;
					for (auto [rule, index] : indexed(indirect.Keys()))
					{
						auto&& startRules1 = indirect.GetByIndex(index);
						for (auto [startRule1, clause1] : startRules1)
						{
							if (rule == startRule1) continue;
							vint index2 = direct.Keys().IndexOf(startRule1);
							if (index2 != -1)
							{
								auto&& startRules2 = direct.GetByIndex(index2);
								for (auto [startRule2, clause2] : startRules2)
								{
									if (rule == startRule2 || startRule1 == startRule2) continue;
									if (!pathToLastRules.Contains({ rule,startRule2 }, { startRule1,clause2 }))
									{
										offset++;
										if (!indirect.Contains(rule, { startRule2,clause2 }))
										{
											indirect.Add(rule, { startRule2,clause2 });
										}
										pathToLastRules.Add({ rule,startRule2 }, { startRule1,clause2 });
									}
								}
							}
						}
					}

					if (offset == 0)
					{
						break;
					}
				}
			}

			void CalculateFirstSet_IndirectStartRules(VisitorContext& context)
			{
				CalculateFirstSet_RuleClosure(
					context.directStartRules,
					context.indirectStartRules,
					context.indirectStartPathToLastRules
					);
			}

			void CalculateFirstSet_IndirectSimpleUseRules(VisitorContext& context)
			{
				CalculateFirstSet_RuleClosure(
					context.directSimpleUseRules,
					context.indirectSimpleUseRules,
					context.indirectSimpleUsePathToLastRules
					);
			}

			template<typename TClause>
			void CalculateFirstSet_MoveFromDirectClauses(
				SortedList<TClause*>& clauses,
				Group<RuleSymbol*, TClause*>& indirectClauses,
				Group<RuleSymbol*, TClause*>& directClauses,
				RuleSymbol* rule,
				RuleSymbol* startRule
			)
			{
				vint index = directClauses.Keys().IndexOf(startRule);
				if (index != -1)
				{
					for (auto clause : directClauses.GetByIndex(index))
					{
						if (!clauses.Contains(clause))
						{
							clauses.Add(clause);
							indirectClauses.Add(rule, clause);
						}
					}
				}
			}

			void CalculateFirstSet_IndirectLrpPmClauses(VisitorContext& context)
			{
				for (auto [rule, index] : indexed(context.indirectStartRules.Keys()))
				{
					SortedList<GlrLeftRecursionInjectClause*> lriClauses;
					SortedList<GlrLeftRecursionPlaceholderClause*> lrpClauses;
					SortedList<GlrPrefixMergeClause*> pmClauses;

					CalculateFirstSet_MoveFromDirectClauses(lriClauses, context.indirectLriClauses, context.directLriClauses, rule, rule);
					CalculateFirstSet_MoveFromDirectClauses(lrpClauses, context.indirectLrpClauses, context.directLrpClauses, rule, rule);
					CalculateFirstSet_MoveFromDirectClauses(pmClauses, context.indirectPmClauses, context.directPmClauses, rule, rule);

					auto&& startRules = context.indirectStartRules.GetByIndex(index);

					for (auto startRule : startRules)
					{
						CalculateFirstSet_MoveFromDirectClauses(lriClauses, context.indirectLriClauses, context.directLriClauses, rule, startRule.ruleSymbol);
						CalculateFirstSet_MoveFromDirectClauses(lrpClauses, context.indirectLrpClauses, context.directLrpClauses, rule, startRule.ruleSymbol);
						CalculateFirstSet_MoveFromDirectClauses(pmClauses, context.indirectPmClauses, context.directPmClauses, rule, startRule.ruleSymbol);
					}
				}
			}

			void CalculateFirstSet(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				CalculateFirstSet_DirectStartRules(context, syntaxFile);
				CalculateFirstSet_IndirectStartRules(context);
				CalculateFirstSet_IndirectSimpleUseRules(context);
				CalculateFirstSet_IndirectLrpPmClauses(context);
			}
		}
	}
}