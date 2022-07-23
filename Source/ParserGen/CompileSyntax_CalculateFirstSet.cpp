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
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			private:
				bool						couldBeEmpty = false;

			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;

				void AddStartRule(const WString& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name);
					if (index != -1)
					{
						context.directStartRules.Add(ruleSymbol, context.syntaxManager.Rules().Values()[index]);
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

			void CalculateFirstSet(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				// calculate directStartRules
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						DirectFirstSetVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							clause->Accept(&visitor);
						}
					}
				}

				// calculate indirectStartRules
				{
					Array<vint> lastCounters(context.directStartRules.Count());
					Array<vint> currentCounters(context.directStartRules.Count());
					for (auto [rule, index] : indexed(context.directStartRules.Keys()))
					{
						auto&& startRules = context.directStartRules.GetByIndex(index);
						lastCounters[index] = 0;
						currentCounters[index] = startRules.Count();
						for (auto startRule : startRules)
						{
							context.indirectStartRules.Add(rule, startRule);
							context.indirectStartRulePairs.Add({ rule,startRule });
						}
					}

					while (true)
					{
						vint offset = 0;
						for (auto [rule, index] : indexed(context.indirectStartRules.Keys()))
						{
							auto&& startRules = context.indirectStartRules.GetByIndex(index);
							vint last = lastCounters[index];
							vint current = currentCounters[index];
							for (vint indexSR = last; indexSR < current; indexSR++)
							{
								auto startRule = startRules[indexSR];
								vint index2 = context.indirectStartRules.Keys().IndexOf(startRule);
								if (index2 != -1 && index2 != index)
								{
									auto&& startRules2 = context.indirectStartRules.GetByIndex(index2);
									vint last2 = lastCounters[index2];
									vint current2 = currentCounters[index2];
									for (vint indexSR2 = last2; indexSR2 < current2; indexSR2++)
									{
										auto startRule2 = startRules2[indexSR2];
										if (!context.indirectStartRulePairs.Contains({ rule,startRule2 }))
										{
											offset++;
											context.indirectStartRules.Add(rule, startRule2);
											context.indirectStartRulePairs.Add({ rule,startRule2 });
										}
									}
								}
							}
						}

						if (offset == 0)
						{
							break;
						}

						for (vint index = 0; index < context.indirectStartRules.Count(); index++)
						{
							auto&& startRules = context.indirectStartRules.GetByIndex(index);
							lastCounters[index] = currentCounters[index];
							currentCounters[index] = startRules.Count();
						}
					}
				}

				// calculate indirectLrpClauses and indirectPmClauses
				for (auto [rule, index] : indexed(context.indirectStartRules.Keys()))
				{
					SortedList<GlrLeftRecursionPlaceholderClause*> lrpClauses;
					SortedList<GlrPrefixMergeClause*> pmClauses;
					auto&& startRules = context.indirectStartRules.GetByIndex(index);

					for (auto startRule : startRules)
					{
						{
							vint indexLrp = context.directLrpClauses.Keys().IndexOf(startRule);
							if (indexLrp != -1)
							{
								for (auto lrp : context.directLrpClauses.GetByIndex(indexLrp))
								{
									if (!lrpClauses.Contains(lrp))
									{
										lrpClauses.Add(lrp);
										context.indirectLrpClauses.Add(rule, lrp);
									}
								}
							}
						}
						{
							vint indexPm = context.directPmClauses.Keys().IndexOf(startRule);
							if (indexPm != -1)
							{
								for (auto pm : context.directPmClauses.GetByIndex(indexPm))
								{
									if (!pmClauses.Contains(pm))
									{
										pmClauses.Add(pm);
										context.indirectPmClauses.Add(rule, pm);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}