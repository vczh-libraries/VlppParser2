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
						if (!context.directStartRules.Contains(ruleSymbol, startRule))
						{
							context.directStartRules.Add(ruleSymbol, startRule);
						}
						if (ruleSymbol == startRule && !context.leftRecursiveClauseParis.Contains({ ruleSymbol,currentClause }))
						{
							context.leftRecursiveClauses.Add(ruleSymbol, currentClause);
							context.leftRecursiveClauseParis.Add({ ruleSymbol,currentClause });
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
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					bool emptyBranch = false;
					for (auto branch : node->branches)
					{
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
						else
						{
							emptyBranch = true;
						}
					}
					couldBeEmpty = emptyBranch;
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
						if (auto useSyntax = dynamic_cast<GlrUseSyntax*>(node->syntax.Obj()))
						{
							if (auto startRule = TryGetRuleSymbol(useSyntax->name.value))
							{
								if (!context.directSimpleUseRules.Contains(ruleSymbol, startRule))
								{
									context.directSimpleUseRules.Add(ruleSymbol, startRule);
								}
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

			void CalculateFirstSet_DirectStartRules(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						DirectFirstSetVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.VisitClause(clause);
						}
					}
				}
			}

			void CalculateFirstSet_RuleClosure(RuleDependencies& direct, RuleDependencies& indirect, StartRuleSet& indirectPairs)
			{
				for (auto [rule, index] : indexed(direct.Keys()))
				{
					auto&& startRules = direct.GetByIndex(index);
					for (auto startRule : startRules)
					{
						indirect.Add(rule, startRule);
						indirectPairs.Add({ rule,startRule });
					}
				}

				while (true)
				{
					vint offset = 0;
					for (auto [rule, index] : indexed(indirect.Keys()))
					{
						auto&& startRules = indirect.GetByIndex(index);
						for (auto startRule : startRules)
						{
							vint index2 = indirect.Keys().IndexOf(startRule);
							if (index2 != -1 && index2 != index)
							{
								auto&& startRules2 = indirect.GetByIndex(index2);
								for (auto startRule2 : startRules2)
								{
									if (!indirectPairs.Contains({ rule,startRule2 }))
									{
										offset++;
										indirect.Add(rule, startRule2);
										indirectPairs.Add({ rule,startRule2 });
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
					context.indirectStartRulePairs
					);
			}

			void CalculateFirstSet_IndirectSimpleUseRules(VisitorContext& context)
			{
				CalculateFirstSet_RuleClosure(
					context.directSimpleUseRules,
					context.indirectSimpleUseRules,
					context.indirectSimpleUseRulePairs
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
						CalculateFirstSet_MoveFromDirectClauses(lriClauses, context.indirectLriClauses, context.directLriClauses, rule, startRule);
						CalculateFirstSet_MoveFromDirectClauses(lrpClauses, context.indirectLrpClauses, context.directLrpClauses, rule, startRule);
						CalculateFirstSet_MoveFromDirectClauses(pmClauses, context.indirectPmClauses, context.directPmClauses, rule, startRule);
					}
				}
			}

			void CalculateFirstSet(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				CalculateFirstSet_DirectStartRules(context, files);
				CalculateFirstSet_IndirectStartRules(context);
				CalculateFirstSet_IndirectSimpleUseRules(context);
				CalculateFirstSet_IndirectLrpPmClauses(context);
			}
		}
	}
}